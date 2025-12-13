#include "PreviewAttachTool.h"
#include "Engine/StaticMesh.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"

#include "ContentBrowserModule.h"
#include "IContentBrowserSingleton.h"
#include "AssetRegistry/AssetData.h"
#include "Modules/ModuleManager.h"
#include "Framework/Application/SlateApplication.h"
#include "Widgets/SWindow.h"
#include "Editor.h"
#include "Editor/EditorEngine.h"
#include "Editor/UnrealEdEngine.h"
#include "Editor/UnrealEd/Public/EditorViewportClient.h"
#include "TickableEditorObject.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Engine/StaticMeshSocket.h"
#include "Framework/Notifications/NotificationManager.h"
#include "Widgets/Notifications/SNotificationList.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Input/SButton.h"
#include "Subsystems/AssetEditorSubsystem.h"
#include "Animation/DebugSkelMeshComponent.h"
#include "Containers/Ticker.h"
#include "Widgets/Input/SNumericEntryBox.h"
#include "Widgets/Input/SSpinBox.h"
#include "Widgets/Docking/SDockTab.h"
#include "Framework/Docking/TabManager.h"

// Include runtime utility library header from the game module
#include "Misc/UtilitiesLibrary.h"

#define LOCTEXT_NAMESPACE "PreviewAttachTool"

// Globals to track spawned preview components and their updaters for removal
static TArray<TSharedPtr<class FPreviewUpdater>> GActivePreviewUpdaters;
static TArray<TWeakObjectPtr<UStaticMeshComponent>> GPreviewComponents;

// Dockable tab id for the Handle Socket editor
static const FName GHandleEditorTabId(TEXT("SocketPreview_HandleEditor"));

// Persistent state for the Handle editor
static TWeakObjectPtr<UStaticMesh> GHandle_StaticMesh;
static FName GHandle_SocketName = FName(TEXT("Handle"));
static TWeakObjectPtr<UStaticMeshComponent> GHandle_PreviewComp;
static TWeakObjectPtr<USkeletalMeshComponent> GHandle_PersonaComp;
static FName GHandle_ParentSocketName = FName(TEXT("Grip"));

// Helper: perform alignment like UtilitiesLibrary but without attaching at the end (keeps preview free)
static void AlignComponentsSocketToSocket_NoAttach(USceneComponent* Target, USceneComponent* Parent, FName TargetSocketName, FName ParentSocketName, bool bScaleToParentSocket)
{
    if (!Target || !Parent) return;
    const FVector OriginalTargetSocketLocation = Target->GetSocketLocation(TargetSocketName);
    const FQuat DeltaRootQuat = Parent->GetSocketQuaternion(ParentSocketName) * Target->GetSocketQuaternion(TargetSocketName).Inverse();
    Target->SetWorldRotation(DeltaRootQuat * Target->GetComponentQuat(), false, nullptr, ETeleportType::TeleportPhysics);
    const FVector RotationLocationOffset = OriginalTargetSocketLocation - Target->GetSocketLocation(TargetSocketName);
    Target->AddWorldOffset(RotationLocationOffset, false, nullptr, ETeleportType::TeleportPhysics);
    Target->AddWorldOffset(Parent->GetSocketLocation(ParentSocketName) - Target->GetSocketLocation(TargetSocketName));
    if (bScaleToParentSocket)
    {
        Target->SetWorldScale3D(Parent->GetSocketTransform(ParentSocketName, RTS_World).GetScale3D());
    }
}

// Helper: try to read the currently selected socket name from the active Persona editor
static FName GetCurrentlySelectedSocketName(USkeletalMeshComponent* PersonaComp)
{
    // Persona selection access is not reliable via public APIs here; return None and use fallbacks below.
    return NAME_None;
}

// Live updater that uses the core ticker to update alignment
class FPreviewUpdater : public TSharedFromThis<FPreviewUpdater>
{
public:
    FPreviewUpdater(UStaticMeshComponent* InPreviewComp, USkeletalMeshComponent* InPersonaComp, FName InTargetSocket, FName InParentSocket)
        : PreviewComp(InPreviewComp), PersonaComp(InPersonaComp), TargetSocket(InTargetSocket), ParentSocket(InParentSocket)
    {}

    void Start()
    {
        if (!TickerHandle.IsValid())
        {
            TickerHandle = FTSTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateRaw(this, &FPreviewUpdater::Tick), 0.0f);
        }
    }

    ~FPreviewUpdater()
    {
        if (TickerHandle.IsValid())
        {
            FTSTicker::GetCoreTicker().RemoveTicker(TickerHandle);
        }
    }

    bool Tick(float DeltaTime)
    {
        if (!PreviewComp.IsValid() || !PersonaComp.IsValid())
        {
            return true; // keep ticking; will no-op until cleaned up externally
        }

        // Align without attaching to keep preview free-standing in the preview world
        AlignComponentsSocketToSocket_NoAttach(
            PreviewComp.Get(),
            PersonaComp.Get(),
            TargetSocket,
            ParentSocket,
            true
        );
        return true;
    }

    TWeakObjectPtr<UStaticMeshComponent> GetPreviewComp() const { return PreviewComp; }

private:
    FTSTicker::FDelegateHandle TickerHandle;
    TWeakObjectPtr<UStaticMeshComponent> PreviewComp;
    TWeakObjectPtr<USkeletalMeshComponent> PersonaComp;
    FName TargetSocket;
    FName ParentSocket;
};

USkeletalMeshComponent* FPreviewAttachTool::GetActivePersonaPreviewMesh()
{
    // Heuristic: among all preview-world skeletal mesh components, prefer one that has a socket named "Grip",
    // then one that has any sockets, otherwise fallback to the first found.
    USkeletalMeshComponent* FirstPreviewComp = nullptr;
    USkeletalMeshComponent* WithAnySocket = nullptr;
    const FName PreferredGrip(TEXT("Grip"));

    for (TObjectIterator<USkeletalMeshComponent> CompIt; CompIt; ++CompIt)
    {
        USkeletalMeshComponent* Comp = *CompIt;
        if (!IsValid(Comp) || !Comp->GetWorld() || !Comp->GetWorld()->IsPreviewWorld())
        {
            continue;
        }

        if (!FirstPreviewComp)
        {
            FirstPreviewComp = Comp;
        }

        TArray<FName> Names = Comp->GetAllSocketNames();
        if (Names.Contains(PreferredGrip))
        {
            return Comp;
        }
        if (!WithAnySocket && Names.Num() > 0)
        {
            WithAnySocket = Comp;
        }
    }

    if (WithAnySocket)
    {
        return WithAnySocket;
    }
    return FirstPreviewComp;
}

void FPreviewAttachTool::OnOpenAssetPickerForCurrentPersona()
{
    // Get Persona's active preview skeletal mesh component
    USkeletalMeshComponent* PersonaComp = GetActivePersonaPreviewMesh();
    if (!PersonaComp)
    {
        // Try to notify user
        FNotificationInfo Info(FText::FromString(TEXT("No Persona preview skeletal mesh found. Open the Skeleton editor (Persona) for a skeletal mesh first.")));
        Info.ExpireDuration = 5.0f;
        FSlateNotificationManager::Get().AddNotification(Info);
        return;
    }

    // Determine parent socket from Persona: use currently selected socket if available
    FName SelectedSocketName = GetCurrentlySelectedSocketName(PersonaComp);

    // Fallbacks: prefer "Grip" -> first available -> finally force "Grip" name
    if (SelectedSocketName.IsNone())
    {
        const FName PreferredGripName(TEXT("Grip"));
        TArray<FName> AllSocketNames = PersonaComp->GetAllSocketNames();
        if (AllSocketNames.Num() > 0)
        {
            if (AllSocketNames.Contains(PreferredGripName))
            {
                SelectedSocketName = PreferredGripName;
            }
            else
            {
                SelectedSocketName = AllSocketNames[0];
            }
        }

        // If still none (no sockets reported), force to "Grip" as requested default
        if (SelectedSocketName.IsNone())
        {
            SelectedSocketName = PreferredGripName;
        }
    }

    // Prepare an asset picker config that lists only Static Mesh assets
    FAssetPickerConfig AssetPickerConfig;
    AssetPickerConfig.Filter.ClassPaths.Add(UStaticMesh::StaticClass()->GetClassPathName());
    AssetPickerConfig.Filter.bRecursiveClasses = true;
    AssetPickerConfig.SelectionMode = ESelectionMode::Single;
    AssetPickerConfig.InitialAssetViewType = EAssetViewType::List;
    AssetPickerConfig.bAllowNullSelection = false;


    // Filter extra safety: only Static Mesh assets
    AssetPickerConfig.OnShouldFilterAsset = FOnShouldFilterAsset::CreateLambda([](const FAssetData& InAssetData)
    {
        return !InAssetData.GetClass()->IsChildOf(UStaticMesh::StaticClass());
    });

    // Prepare a safe, self-closing selection handler by capturing a shared weak reference to the window
    TSharedRef<TWeakPtr<SWindow>> PickerWindowWeakRef = MakeShared<TWeakPtr<SWindow>>();

    AssetPickerConfig.OnAssetSelected = FOnAssetSelected::CreateLambda([PersonaComp, SelectedSocketName, PickerWindowWeakRef](const FAssetData& InAssetData)
    {
        FPreviewAttachTool::OnStaticMeshPicked(InAssetData, PersonaComp, SelectedSocketName);
        if (TSharedPtr<SWindow> WindowPinned = PickerWindowWeakRef->Pin())
        {
            WindowPinned->RequestDestroyWindow();
        }
    });

    FContentBrowserModule& ContentBrowserModule = FModuleManager::LoadModuleChecked<FContentBrowserModule>("ContentBrowser");
    TSharedRef<SWidget> AssetPicker = ContentBrowserModule.Get().CreateAssetPicker(AssetPickerConfig);

    // Create a window for the picker (no custom buttons; double-click or Enter selects and closes)
    TSharedRef<SWindow> PickerWindow = SNew(SWindow)
        .Title(LOCTEXT("PickStaticMesh", "Pick StaticMesh for Socket Preview"))
        .ClientSize(FVector2D(1000, 600))
        .SupportsMinimize(false).SupportsMaximize(false)
        [
            SNew(SVerticalBox)
            + SVerticalBox::Slot().FillHeight(1.0f)[ AssetPicker ]
        ];

    // Set the weak ref so the selection handler can close the window
    *PickerWindowWeakRef = PickerWindow;

    FSlateApplication::Get().AddWindow(PickerWindow);
}

// Custom widget: a label that supports horizontal drag to adjust a numeric value without clamping
class SDraggableLabel : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SDraggableLabel){}
		SLATE_ATTRIBUTE(FText, Text)
		SLATE_EVENT(FSimpleDelegate, OnBeginDrag)
		SLATE_EVENT(FSimpleDelegate, OnEndDrag)
		SLATE_EVENT(FOnFloatValueChanged, OnDelta)
		SLATE_ARGUMENT(float, Sensitivity)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs)
	{
		LabelText = InArgs._Text;
		OnBeginDrag = InArgs._OnBeginDrag;
		OnEndDrag = InArgs._OnEndDrag;
		OnDelta = InArgs._OnDelta;
		Sensitivity = InArgs._Sensitivity == 0.f ? 0.1f : InArgs._Sensitivity;

		ChildSlot
		[
			SNew(STextBlock)
			.Text(LabelText)
			.ToolTipText(LOCTEXT("DragTip", "Click and drag to adjust"))
			.Cursor(EMouseCursor::ResizeLeftRight)
		];
	}

	virtual FReply OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override
	{
		if (MouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
		{
			bDragging = true;
			LastCursorPos = MouseEvent.GetScreenSpacePosition();
			if (OnBeginDrag.IsBound()) OnBeginDrag.Execute();
			return FReply::Handled().CaptureMouse(SharedThis(this));
		}
		return FReply::Unhandled();
	}

	virtual FReply OnMouseButtonUp(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override
	{
		if (bDragging && MouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
		{
			bDragging = false;
			if (OnEndDrag.IsBound()) OnEndDrag.Execute();
			return FReply::Handled().ReleaseMouseCapture();
		}
		return FReply::Unhandled();
	}

	virtual FReply OnMouseMove(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override
	{
		if (bDragging && OnDelta.IsBound())
		{
			const FVector2D Now = MouseEvent.GetScreenSpacePosition();
			const float DX = Now.X - LastCursorPos.X;
			LastCursorPos = Now;
			OnDelta.Execute(DX * Sensitivity);
			return FReply::Handled();
		}
		return FReply::Unhandled();
	}

	virtual bool SupportsKeyboardFocus() const override { return false; }

private:
	TAttribute<FText> LabelText;
	FSimpleDelegate OnBeginDrag;
	FSimpleDelegate OnEndDrag;
	FOnFloatValueChanged OnDelta;
	FVector2D LastCursorPos = FVector2D::ZeroVector;
	bool bDragging = false;
	float Sensitivity = 0.1f;
};

// Helper: find the Handle socket on the current static mesh
static UStaticMeshSocket* GetCurrentHandleSocket()
{
	if (!GHandle_StaticMesh.IsValid()) return nullptr;
	for (UStaticMeshSocket* Sock : GHandle_StaticMesh->Sockets)
	{
		if (Sock && Sock->SocketName == GHandle_SocketName)
		{
			return Sock;
		}
	}
	return nullptr;
}

// Helper: apply changes and realign preview
static void ApplyAndRefresh_Current()
{
	if (!GHandle_StaticMesh.IsValid()) return;
	GHandle_StaticMesh->Modify();
	GHandle_StaticMesh->PostEditChange();
	if (UPackage* Pkg = GHandle_StaticMesh->GetOutermost())
	{
		Pkg->SetDirtyFlag(true);
	}
	if (GHandle_PreviewComp.IsValid() && GHandle_PersonaComp.IsValid())
	{
		AlignComponentsSocketToSocket_NoAttach(GHandle_PreviewComp.Get(), GHandle_PersonaComp.Get(), GHandle_SocketName, GHandle_ParentSocketName, true);
	}
}

TSharedRef<SWidget> FPreviewAttachTool::CreateHandleSocketEditorWidget()
{
	// Cached values (read initial from socket if available)
	float LocX = 0.f, LocY = 0.f, LocZ = 0.f, Pitch = 0.f, Yaw = 0.f, Roll = 0.f;
	if (UStaticMeshSocket* S = GetCurrentHandleSocket())
	{
		LocX = S->RelativeLocation.X;
		LocY = S->RelativeLocation.Y;
		LocZ = S->RelativeLocation.Z;
		Pitch = S->RelativeRotation.Pitch;
		Yaw = S->RelativeRotation.Yaw;
		Roll = S->RelativeRotation.Roll;
	}

	auto MakeRow = [&](const FText& Label, TFunction<float()> Getter, TFunction<void(float)> Setter)
	{
		return SNew(SHorizontalBox)
		+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(4,2)
		[
			SNew(SDraggableLabel)
			.Text(Label)
			.Sensitivity(0.25f)
			.OnDelta(FOnFloatValueChanged::CreateLambda([Setter, Getter](float Delta){ Setter(Getter() + Delta); ApplyAndRefresh_Current(); }))
		]
		+ SHorizontalBox::Slot().FillWidth(1.f).Padding(4,2).VAlign(VAlign_Center)
		[
			SNew(SSpinBox<float>)
			.Value_Lambda([Getter](){ return Getter(); })
			.OnValueChanged_Lambda([Setter](float V){ Setter(V); ApplyAndRefresh_Current(); })
			.OnValueCommitted_Lambda([Setter](float V, ETextCommit::Type){ Setter(V); ApplyAndRefresh_Current(); })
			.Delta(0.5f)
			.MinValue(TOptional<float>()) // unbounded
			.MaxValue(TOptional<float>())
			.MinSliderValue(TOptional<float>())
			.MaxSliderValue(TOptional<float>())
			.SupportDynamicSliderMaxValue(false)
		];
	};

	auto GetLocX = [&]() -> float { if (auto* S = GetCurrentHandleSocket()) return S->RelativeLocation.X; return LocX; };
	auto GetLocY = [&]() -> float { if (auto* S = GetCurrentHandleSocket()) return S->RelativeLocation.Y; return LocY; };
	auto GetLocZ = [&]() -> float { if (auto* S = GetCurrentHandleSocket()) return S->RelativeLocation.Z; return LocZ; };
	auto GetPitch = [&]() -> float { if (auto* S = GetCurrentHandleSocket()) return S->RelativeRotation.Pitch; return Pitch; };
	auto GetYaw = [&]() -> float { if (auto* S = GetCurrentHandleSocket()) return S->RelativeRotation.Yaw; return Yaw; };
	auto GetRoll = [&]() -> float { if (auto* S = GetCurrentHandleSocket()) return S->RelativeRotation.Roll; return Roll; };

	auto SetLocX = [&](float V){ if (auto* S = GetCurrentHandleSocket()){ S->RelativeLocation.X = V; } LocX = V; };
	auto SetLocY = [&](float V){ if (auto* S = GetCurrentHandleSocket()){ S->RelativeLocation.Y = V; } LocY = V; };
	auto SetLocZ = [&](float V){ if (auto* S = GetCurrentHandleSocket()){ S->RelativeLocation.Z = V; } LocZ = V; };
	auto SetPitch = [&](float V){ if (auto* S = GetCurrentHandleSocket()){ auto R=S->RelativeRotation; R.Pitch=V; S->RelativeRotation=R; } Pitch = V; };
	auto SetYaw   = [&](float V){ if (auto* S = GetCurrentHandleSocket()){ auto R=S->RelativeRotation; R.Yaw=V; S->RelativeRotation=R; } Yaw = V; };
	auto SetRoll  = [&](float V){ if (auto* S = GetCurrentHandleSocket()){ auto R=S->RelativeRotation; R.Roll=V; S->RelativeRotation=R; } Roll = V; };

	return SNew(SVerticalBox)
	+ SVerticalBox::Slot().AutoHeight().Padding(6)
	[
		SNew(STextBlock).Text(LOCTEXT("HandleSocketHelp", "Adjust the Handle socket (dock this tab next to Details). Drag labels or type values."))
	]
	+ SVerticalBox::Slot().AutoHeight().Padding(6)[ MakeRow(LOCTEXT("LocX","Location X"), GetLocX, SetLocX) ]
	+ SVerticalBox::Slot().AutoHeight().Padding(6)[ MakeRow(LOCTEXT("LocY","Location Y"), GetLocY, SetLocY) ]
	+ SVerticalBox::Slot().AutoHeight().Padding(6)[ MakeRow(LOCTEXT("LocZ","Location Z"), GetLocZ, SetLocZ) ]
	+ SVerticalBox::Slot().AutoHeight().Padding(6)[ MakeRow(LOCTEXT("Pitch","Pitch"), GetPitch, SetPitch) ]
	+ SVerticalBox::Slot().AutoHeight().Padding(6)[ MakeRow(LOCTEXT("Yaw","Yaw"), GetYaw, SetYaw) ]
	+ SVerticalBox::Slot().AutoHeight().Padding(6)[ MakeRow(LOCTEXT("Roll","Roll"), GetRoll, SetRoll) ]
	+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Right).Padding(6)
	[
		SNew(SButton)
		.Text(LOCTEXT("ResetBtn","Reset"))
		.OnClicked_Lambda([]() -> FReply {
			if (auto* S = GetCurrentHandleSocket())
			{
				S->RelativeLocation = FVector::ZeroVector;
				S->RelativeRotation = FRotator::ZeroRotator;
				ApplyAndRefresh_Current();
			}
			return FReply::Handled();
		})
	];
}

void FPreviewAttachTool::OnStaticMeshPicked(const FAssetData& AssetData, TWeakObjectPtr<USkeletalMeshComponent> PersonaPreviewMeshComp, FName SelectedSocketName)
{
    UObject* AssetObj = AssetData.GetAsset();
    if (!AssetObj) return;

    UStaticMesh* StaticMesh = Cast<UStaticMesh>(AssetObj);
    if (!StaticMesh) return;

    USkeletalMeshComponent* PersonaComp = PersonaPreviewMeshComp.IsValid() ? PersonaPreviewMeshComp.Get() : GetActivePersonaPreviewMesh();
    if (!PersonaComp) return;

    // Parent socket should be the constant "Grip"
    const FName ParentSocketName(TEXT("Grip"));

    // Ensure a target socket named "Handle" exists on the static mesh; create if missing
    const FName PreferredHandleName(TEXT("Handle"));
    UStaticMeshSocket* HandleSocket = nullptr;
    for (UStaticMeshSocket* Sock : StaticMesh->Sockets)
    {
        if (Sock && Sock->SocketName == PreferredHandleName)
        {
            HandleSocket = Sock;
            break;
        }
    }
    if (!HandleSocket)
    {
        StaticMesh->Modify();
        HandleSocket = NewObject<UStaticMeshSocket>(StaticMesh, UStaticMeshSocket::StaticClass(), NAME_None, RF_Transactional);
        HandleSocket->SocketName = PreferredHandleName;
        HandleSocket->RelativeLocation = FVector::ZeroVector;
        HandleSocket->RelativeRotation = FRotator::ZeroRotator;
        StaticMesh->Sockets.Add(HandleSocket);
        StaticMesh->PostEditChange();
        if (UPackage* Pkg = StaticMesh->GetOutermost()) { Pkg->SetDirtyFlag(true); }
    }
    const FName TargetSocketName = PreferredHandleName;

    // Create transient preview static mesh component as a child of the Persona preview component's preview scene root
    if (!PersonaComp->GetWorld())
    {
        return;
    }

    UStaticMeshComponent* PreviewComp = NewObject<UStaticMeshComponent>(PersonaComp);
    PreviewComp->SetFlags(RF_Transient);
    PreviewComp->SetStaticMesh(StaticMesh);
    PreviewComp->SetMobility(EComponentMobility::Movable);
    PreviewComp->RegisterComponentWithWorld(PersonaComp->GetWorld());

    // Attach to Persona preview component root so it is in the same preview scene
    PreviewComp->AttachToComponent(PersonaComp, FAttachmentTransformRules::KeepRelativeTransform);

    // Ensure visible in editor preview
    PreviewComp->SetVisibility(true, true);
    PreviewComp->SetHiddenInGame(false);

    // Optionally zero transform first
    PreviewComp->SetRelativeLocation(FVector::ZeroVector);
    PreviewComp->SetRelativeRotation(FRotator::ZeroRotator);
    PreviewComp->SetRelativeScale3D(FVector::OneVector);

    // Initial alignment so user immediately sees placement (no attach)
    AlignComponentsSocketToSocket_NoAttach(
        PreviewComp,
        PersonaComp,
        TargetSocketName,
        ParentSocketName,
        true
    );

    // Create updater which will call your align function every tick via ticker
    TSharedPtr<FPreviewUpdater> Updater = MakeShared<FPreviewUpdater>(PreviewComp, PersonaComp, TargetSocketName, ParentSocketName);
    Updater->Start();
    GActivePreviewUpdaters.Add(Updater);
    GPreviewComponents.Add(PreviewComp);

    // Update persistent editor state for the dockable tab and open/focus it
    GHandle_StaticMesh = StaticMesh;
    GHandle_SocketName = TargetSocketName;
    GHandle_PreviewComp = PreviewComp;
    GHandle_PersonaComp = PersonaComp;
    GHandle_ParentSocketName = ParentSocketName;

    FGlobalTabmanager::Get()->TryInvokeTab(GHandleEditorTabId);

    // Note: Updaters will persist for the editor session. Consider cleanup on plugin shutdown if needed.
}

#undef LOCTEXT_NAMESPACE

void FPreviewAttachTool::RemoveAllPreviewMeshes()
{
    // Destroy all spawned preview components
    for (int32 Index = GPreviewComponents.Num() - 1; Index >= 0; --Index)
    {
        if (UStaticMeshComponent* Comp = GPreviewComponents[Index].Get())
        {
            if (Comp->IsRegistered())
            {
                Comp->UnregisterComponent();
            }
            Comp->DestroyComponent();
        }
    }
    GPreviewComponents.Empty();

    // Release updaters (their destructors will remove tickers)
    GActivePreviewUpdaters.Empty();

    FNotificationInfo Info(FText::FromString(TEXT("Removed all preview meshes.")));
    Info.ExpireDuration = 2.0f;
    FSlateNotificationManager::Get().AddNotification(Info);
}
