//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////
///		Copyright 2019 (C) Bruno Xavier B. Leite
//////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "KMGC_NodeWidget.h"
#include "KMGC_TextSyntaxHighlighter.h"

#include "Runtime/Core/Public/Misc/CString.h"
#include "Runtime/InputCore/Classes/InputCoreTypes.h"

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define LOCTEXT_NAMESPACE "Synaptech"

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// MGC Main Widget Constructors:

SKMGC_MagicNodeWidget::SKMGC_MagicNodeWidget(){}
SKMGC_MagicNodeWidget::~SKMGC_MagicNodeWidget(){}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// MGC Main Widget:

void SKMGC_MagicNodeWidget::Construct(const FArguments &InArgs, UKMGC_MagicNode* InGraphNode) {
	GraphNode = InGraphNode; if (GraphNode) {UpdateGraphNode();}
}

void SKMGC_MagicNodeWidget::Tick(const FGeometry &AllottedGeometry, const double CurrentTime, const float DeltaTime) {
	SGraphNode::Tick(AllottedGeometry,CurrentTime,DeltaTime);
	//
	HintTimer = (HintTimer>=15.f) ? 0.f : (HintTimer+0.5f);
}

void SKMGC_MagicNodeWidget::CreateBelowWidgetControls(TSharedPtr<SVerticalBox>MainBox) {
	UpdateDatabaseReferences();
	//
	//
	ViewSearchBox = false;
	Source = ESKMGC_Source::Header;
	AutoComplete = EAutoComplete::Off;
	//
	SetLineCountList(GetLineCount());
	SetMacrosList(GetScriptMacros());
	SetIncludesList(GetScriptIncludes());
	//
	//
	VS_SCROLL = SNew(SScrollBar)
	.OnUserScrolled(this,&SKMGC_MagicNodeWidget::OnInternalVerticalScroll)
	.Thickness(FVector2D(8.f,8.f)).AlwaysShowScrollbar(false)
	.Orientation(Orient_Vertical);
	//
	HSS_SCROLL = SNew(SScrollBar)
	.OnUserScrolled(this,&SKMGC_MagicNodeWidget::OnScriptHorizontalScroll)
	.Visibility(this,&SKMGC_MagicNodeWidget::GetScriptPanelVisibility)
	.Orientation(Orient_Horizontal).AlwaysShowScrollbar(false)
	.Thickness(FVector2D(8.f,8.f));
	//
	HSH_SCROLL = SNew(SScrollBar)
	.OnUserScrolled(this,&SKMGC_MagicNodeWidget::OnHeaderHorizontalScroll)
	.Visibility(this,&SKMGC_MagicNodeWidget::GetHeaderPanelVisibility)
	.Orientation(Orient_Horizontal).AlwaysShowScrollbar(false)
	.Thickness(FVector2D(8.f,8.f));
	//
	AUTOCOMPLETE_SCROLL = SNew(SScrollBar)
	.OnUserScrolled(this,&SKMGC_MagicNodeWidget::OnInternalAutoCompleteScroll)
	.Thickness(FVector2D(8.f,8.f)).AlwaysShowScrollbar(false)
	.Orientation(Orient_Vertical);
	//
	//
	MainBox->AddSlot()
	.VAlign(VAlign_Top).HAlign(HAlign_Fill)
	[
	SNew(SHorizontalBox)
	+SHorizontalBox::Slot().HAlign(HAlign_Fill)
	[
		SNew(SBorder).Padding(1)
		.Visibility(this,&SKMGC_MagicNodeWidget::GetScriptEditorVisibility)
		.BorderImage(FEditorStyle::GetBrush("Graph.Node.Body"))
		.ColorAndOpacity(FLinearColor(1.f,1.f,1.f,0.8f))
		[
			SNew(SVerticalBox)
			+SVerticalBox::Slot()
			.AutoHeight().Padding(10,5,10,5)
			[
				SNew(SHorizontalBox)
				+SHorizontalBox::Slot().HAlign(HAlign_Fill)
				[
					SAssignNew(SCRIPT_PICKER,SContentReference)
					.AssetReference(this,&SKMGC_MagicNodeWidget::GetScriptObject)
					.OnSetReference(this,&SKMGC_MagicNodeWidget::SetScriptObject)
					.AllowedClass(UMagicNodeScript::StaticClass())
					.AssetPickerSizeOverride(FVector2D(540.f,360.f))
					.ShowFindInBrowserButton(true)
					.AllowClearingReference(true)
					.AllowSelectingNewAsset(true)
					.ShowToolsButton(false)
					.WidthOverride(400.f)
				]
			]
			+SVerticalBox::Slot()
			.AutoHeight().Padding(10,0,10,0)
			[
				SNew(SVerticalBox)
				+SVerticalBox::Slot().AutoHeight()
				[
					SNew(SHorizontalBox)
					+SHorizontalBox::Slot().AutoWidth()
					.VAlign(VAlign_Center).HAlign(HAlign_Left)
					[
						SNew(SBorder)
						.VAlign(VAlign_Fill).HAlign(HAlign_Fill)
						.BorderImage(FEditorStyle::GetBrush("Graph.Node.Body"))
						[
							SNew(SButton)
							.OnClicked(this,&SKMGC_MagicNodeWidget::OnClickedAddInclude)
							.ButtonStyle(FEditorStyle::Get(),"NoBorder")
							.VAlign(VAlign_Center).HAlign(HAlign_Center)
							[
								SNew(SImage)
								.Image(FEditorStyle::GetBrush("Plus"))
							]
						]
					]
					+SHorizontalBox::Slot()
					.VAlign(VAlign_Center).HAlign(HAlign_Fill)
					[
						SNew(SBorder)
						.VAlign(VAlign_Fill).HAlign(HAlign_Fill)
						.BorderImage(FEditorStyle::GetBrush("Graph.Node.Body"))
						[
							SNew(SButton)
							.OnClicked(this,&SKMGC_MagicNodeWidget::OnClickedViewIncludes)
							.ButtonStyle(FEditorStyle::Get(),"NoBorder")
							.VAlign(VAlign_Fill).HAlign(HAlign_Fill)
							[
								SNew(STextBlock).Margin(FMargin(5,0,0,0))
								.Text(LOCTEXT("KMGC_IncludesPanel","# Includes"))
								.ColorAndOpacity(FSlateColor(FLinearColor(FColor(155,155,155,225))))
							]
						]
					]
				]
				+SVerticalBox::Slot().AutoHeight()
				[
					SNew(SBorder)
					.VAlign(VAlign_Fill).HAlign(HAlign_Fill)
					.BorderImage(FEditorStyle::GetBrush("Graph.Node.Body"))
					.Visibility(this,&SKMGC_MagicNodeWidget::GetIncludesPanelVisibility)
					[
						SAssignNew(INCLUDES_LIST,SListView<TSharedPtr<FString>>)
						.OnGenerateRow(this,&SKMGC_MagicNodeWidget::OnGenerateIncludes)
						.ScrollbarVisibility(EVisibility::Hidden)
						.SelectionMode(ESelectionMode::Single)
						.ListItemsSource(&SourceIncludes)
					]
				]
				+SVerticalBox::Slot().AutoHeight()
				[
					SNew(SHorizontalBox)
					+SHorizontalBox::Slot().AutoWidth()
					.VAlign(VAlign_Center).HAlign(HAlign_Left)
					[
						SNew(SBorder)
						.VAlign(VAlign_Fill).HAlign(HAlign_Fill)
						.BorderImage(FEditorStyle::GetBrush("Graph.Node.Body"))
						[
							SNew(SButton)
							.OnClicked(this,&SKMGC_MagicNodeWidget::OnClickedAddMacro)
							.ButtonStyle(FEditorStyle::Get(),"NoBorder")
							.VAlign(VAlign_Center).HAlign(HAlign_Center)
							[
								SNew(SImage)
								.Image(FEditorStyle::GetBrush("Plus"))
							]
						]
					]
					+SHorizontalBox::Slot()
					.VAlign(VAlign_Center).HAlign(HAlign_Fill)
					[
						SNew(SBorder)
						.VAlign(VAlign_Fill).HAlign(HAlign_Fill)
						.BorderImage(FEditorStyle::GetBrush("Graph.Node.Body"))
						[
							SNew(SButton)
							.OnClicked(this,&SKMGC_MagicNodeWidget::OnClickedViewMacros)
							.ButtonStyle(FEditorStyle::Get(),"NoBorder")
							.VAlign(VAlign_Fill).HAlign(HAlign_Fill)
							[
								SNew(STextBlock).Margin(FMargin(5,0,0,0))
								.Text(LOCTEXT("KMGC_MacrosPanel","[] Macros"))
								.ColorAndOpacity(FSlateColor(FLinearColor(FColor(155,155,155,225))))
							]
						]
					]
				]
				+SVerticalBox::Slot().AutoHeight()
				[
					SNew(SBorder)
					.VAlign(VAlign_Fill).HAlign(HAlign_Fill)
					.BorderImage(FEditorStyle::GetBrush("Graph.Node.Body"))
					.Visibility(this,&SKMGC_MagicNodeWidget::GetMacrosPanelVisibility)
					[
						SAssignNew(MACROS_LIST,SListView<TSharedPtr<FString>>)
						.OnGenerateRow(this,&SKMGC_MagicNodeWidget::OnGenerateMacros)
						.ScrollbarVisibility(EVisibility::Hidden)
						.SelectionMode(ESelectionMode::Single)
						.ListItemsSource(&SourceMacros)
					]
				]
			]
			+SVerticalBox::Slot()
			.FillHeight(1.0f).Padding(10,5,10,5)
			[
				SNew(SHorizontalBox)
				+SHorizontalBox::Slot()
				.HAlign(HAlign_Left).AutoWidth()
				[
					SNew(SVerticalBox)
					+SVerticalBox::Slot()
					.AutoHeight().Padding(-32,0,0,4)
					[
						SNew(SButton)
						.ButtonColorAndOpacity(this,&SKMGC_MagicNodeWidget::GetHeaderIconColor)
						.OnClicked(this,&SKMGC_MagicNodeWidget::OnClickedHeaderIcon)
						.VAlign(VAlign_Top).HAlign(HAlign_Left)
						[
							SNew(SImage)
							.Image(FKMGC_NodeStyle::Get()->GetBrush(TEXT("KMGC.Toolbar.H")))
						]
					]
					+SVerticalBox::Slot()
					.AutoHeight().Padding(-32,0,0,4)
					[
						SNew(SButton)
						.ButtonColorAndOpacity(this,&SKMGC_MagicNodeWidget::GetScriptIconColor)
						.OnClicked(this,&SKMGC_MagicNodeWidget::OnClickedScriptIcon)
						.VAlign(VAlign_Top).HAlign(HAlign_Left)
						[
							SNew(SImage)
							.Image(FKMGC_NodeStyle::Get()->GetBrush(TEXT("KMGC.Toolbar.CPP")))
						]
					]
					+SVerticalBox::Slot()
					.AutoHeight().Padding(-32,0,0,4)
					[
						SNew(SButton)
						.ButtonColorAndOpacity(this,&SKMGC_MagicNodeWidget::GetTypesIconColor)
						.OnClicked(this,&SKMGC_MagicNodeWidget::OnClickedTypesIcon)
						.VAlign(VAlign_Top).HAlign(HAlign_Left)
						[
							SNew(SImage)
							.Image(FKMGC_NodeStyle::Get()->GetBrush(TEXT("KMGC.Toolbar.T")))
						]
					]
				]
				+SHorizontalBox::Slot().HAlign(HAlign_Fill)
				[
					SNew(SOverlay)
					+SOverlay::Slot()
					.VAlign(VAlign_Fill).HAlign(HAlign_Fill)
					[
						SNew(SBox)
						.VAlign(VAlign_Fill).HAlign(HAlign_Fill)
						.MinDesiredWidth(400.f).MinDesiredHeight(250.f)
						.MaxDesiredHeight(1010.f).MaxDesiredWidth(1010.f)
						[
							SNew(SBorder)
							.VAlign(VAlign_Fill).HAlign(HAlign_Fill)
							.BorderImage(FEditorStyle::GetBrush("ToolPanel.DarkGroupBorder"))
							[
								SAssignNew(VS_SCROLL_BOX,SScrollBox)
								.OnUserScrolled(this,&SKMGC_MagicNodeWidget::OnVerticalScroll)
								.Orientation(EOrientation::Orient_Vertical)
								.ScrollBarThickness(FVector2D(8.f,8.f))
								+SScrollBox::Slot()
								[
									SNew(SHorizontalBox)
									+SHorizontalBox::Slot()
									.VAlign(VAlign_Fill).HAlign(HAlign_Left).AutoWidth()
									[
										SNew(SBorder)
										.VAlign(VAlign_Fill).HAlign(HAlign_Fill)
										.BorderImage(FEditorStyle::GetBrush("Graph.Node.Body"))
										[
											SAssignNew(LINE_COUNTER,SListView<TSharedPtr<FString>>)
											.OnSelectionChanged(this,&SKMGC_MagicNodeWidget::OnSelectedLineCounterItem)
											.OnGenerateRow(this,&SKMGC_MagicNodeWidget::OnGenerateLineCounter)
											.ScrollbarVisibility(EVisibility::Collapsed)
											.ListItemsSource(&LineCount).ItemHeight(14)
											.SelectionMode(ESelectionMode::Single)
										]
									]
									+SHorizontalBox::Slot()
									.VAlign(VAlign_Fill).HAlign(HAlign_Fill)
									[
										SAssignNew(SCRIPT_EDITOR,SKMGC_TextEditorWidget)
										.OnTextChanged(this,&SKMGC_MagicNodeWidget::OnScriptTextChanged,ETextCommit::Default)
										.OnTextCommitted(this,&SKMGC_MagicNodeWidget::OnScriptTextComitted)
										.Visibility(this,&SKMGC_MagicNodeWidget::GetScriptPanelVisibility)
										.OnInvokeSearch(this,&SKMGC_MagicNodeWidget::OnInvokedSearch)
										.OnAutoComplete(this,&SKMGC_MagicNodeWidget::OnAutoComplete)
										.IsEnabled(this,&SKMGC_MagicNodeWidget::HasScript)
										.Text(this,&SKMGC_MagicNodeWidget::GetScriptText)
										.VScrollBar(VS_SCROLL).HScrollBar(HSS_SCROLL)
										.Marshaller(MARSHALL.ToSharedRef())
										.CanKeyboardFocus(true)
										.IsReadOnly(false)
									]
									+SHorizontalBox::Slot()
									.VAlign(VAlign_Fill).HAlign(HAlign_Fill)
									[
										SAssignNew(HEADER_EDITOR,SKMGC_TextEditorWidget)
										.OnTextChanged(this,&SKMGC_MagicNodeWidget::OnHeaderTextChanged,ETextCommit::Default)
										.OnTextCommitted(this,&SKMGC_MagicNodeWidget::OnHeaderTextComitted)
										.Visibility(this,&SKMGC_MagicNodeWidget::GetHeaderPanelVisibility)
										.OnInvokeSearch(this,&SKMGC_MagicNodeWidget::OnInvokedSearch)
										.OnAutoComplete(this,&SKMGC_MagicNodeWidget::OnAutoComplete)
										.IsEnabled(this,&SKMGC_MagicNodeWidget::HasScript)
										.Text(this,&SKMGC_MagicNodeWidget::GetHeaderText)
										.VScrollBar(VS_SCROLL).HScrollBar(HSH_SCROLL)
										.Marshaller(MARSHALL.ToSharedRef())
										.CanKeyboardFocus(true)
										.IsReadOnly(false)
									]
									+SHorizontalBox::Slot()
									.VAlign(VAlign_Fill).HAlign(HAlign_Fill)
									[
										SAssignNew(TYPES_EDITOR,SKMGC_TextEditorWidget)
										.OnTextChanged(this,&SKMGC_MagicNodeWidget::OnTypesTextChanged,ETextCommit::Default)
										.OnTextCommitted(this,&SKMGC_MagicNodeWidget::OnTypesTextComitted)
										.Visibility(this,&SKMGC_MagicNodeWidget::GetTypesPanelVisibility)
										.IsEnabled(this,&SKMGC_MagicNodeWidget::HasScript)
										.Text(this,&SKMGC_MagicNodeWidget::GetTypesText)
										.Marshaller(MARSHALL.ToSharedRef())
										.CanKeyboardFocus(true)
										.IsReadOnly(false)
									]
								]
							]
						]
					]
					+SOverlay::Slot()
					.VAlign(VAlign_Top).HAlign(HAlign_Right)
					[
						SNew(SBox)
						.VAlign(VAlign_Fill).HAlign(HAlign_Fill)
						.WidthOverride(250.f).HeightOverride(80.f)
						.Visibility(this,&SKMGC_MagicNodeWidget::GetSearchBoxVisibility)
						[
							SNew(SBorder)
							.VAlign(VAlign_Fill).HAlign(HAlign_Fill)
							.BorderImage(FEditorStyle::GetBrush("Sequencer.Thumbnail.SectionHandle"))
							[
								SNew(SBorder)
								.VAlign(VAlign_Fill).HAlign(HAlign_Fill)
								.BorderImage(FEditorStyle::GetBrush("Graph.Node.Body"))
								[
									SNew(SVerticalBox)
									+SVerticalBox::Slot().Padding(5)
									.VAlign(VAlign_Top).HAlign(HAlign_Fill)
									[
										SNew(SHorizontalBox)
										+SHorizontalBox::Slot()
										.Padding(0,2,5,0).AutoWidth()
										.VAlign(VAlign_Top).HAlign(HAlign_Left)
										[
											SNew(SCheckBox)
											.OnCheckStateChanged(this,&SKMGC_MagicNodeWidget::OnSearchSensitiveChanged)
											.IsChecked(this,&SKMGC_MagicNodeWidget::IsSearchSensitive)
											.Style(FEditorStyle::Get(),"ToggleButtonCheckbox")
											.Content()
											[
												SNew(STextBlock).Margin(2)
												.Text(LOCTEXT("KMGC_SearchToggleCase","Aa"))
												.ColorAndOpacity(FSlateColor(FLinearColor(FColor(255,255,255,225))))
												.Font(FKMGC_NodeStyle::Get().Get()->GetWidgetStyle<FTextBlockStyle>("KMGC.SourceBlockStyle").Font)
											]
										]
										+SHorizontalBox::Slot()
										.VAlign(VAlign_Fill).HAlign(HAlign_Fill)
										[
											SAssignNew(SEARCH_TEXT,SEditableTextBox)
											.Text(this,&SKMGC_MagicNodeWidget::GetSearchText)
											.OnTextCommitted(this,&SKMGC_MagicNodeWidget::OnSearchTextComitted)
											.OnTextChanged(this,&SKMGC_MagicNodeWidget::OnSearchTextChanged,ETextCommit::Default)
										]
										+SHorizontalBox::Slot()
										.Padding(5,2,0,0).AutoWidth()
										.VAlign(VAlign_Top).HAlign(HAlign_Left)
										[
											SNew(SButton)
											.OnClicked(this,&SKMGC_MagicNodeWidget::OnClickedSearchGlass)
											.ButtonStyle(FEditorStyle::Get(),"NoBorder")
											[
												SNew(SImage)
												.Image(FEditorStyle::Get().GetBrush(TEXT("Symbols.SearchGlass")))
											]
										]
									]
									+SVerticalBox::Slot().Padding(5)
									.VAlign(VAlign_Top).HAlign(HAlign_Fill)
									[
										SNew(SHorizontalBox)
										+SHorizontalBox::Slot()
										.Padding(5,0,5,0).AutoWidth()
										.VAlign(VAlign_Top).HAlign(HAlign_Left)
										[
											SNew(SBox)
											.VAlign(VAlign_Fill).HAlign(HAlign_Fill)
											.WidthOverride(22.f).HeightOverride(22.f)
										]
										+SHorizontalBox::Slot()
										.VAlign(VAlign_Fill).HAlign(HAlign_Fill)
										[
											SNew(SEditableTextBox)
											.Text(this,&SKMGC_MagicNodeWidget::GetReplaceText)
											.OnTextCommitted(this,&SKMGC_MagicNodeWidget::OnReplaceTextComitted)
										]
										+SHorizontalBox::Slot()
										.Padding(5,0,0,5).AutoWidth()
										.VAlign(VAlign_Top).HAlign(HAlign_Left)
										[
											SNew(SButton)
											.OnClicked(this,&SKMGC_MagicNodeWidget::OnClickedReplaceSearch)
											.ButtonStyle(FEditorStyle::Get(),"NoBorder")
											[
												SNew(STextBlock).Margin(2)
												.Text(FText::FromString(FString(TEXT("\xf061"))))
												.Font(FEditorStyle::Get().GetFontStyle("FontAwesome.11"))
												.ColorAndOpacity(FSlateColor(FLinearColor(FColor(255,255,255,225))))
											]
										]
									]
								]
							]
						]
					]
				]
			]
			+SVerticalBox::Slot()
			.AutoHeight().Padding(10,0,10,0)
			[
				SNew(SVerticalBox)
				+SVerticalBox::Slot().AutoHeight()
				[
					HSS_SCROLL.ToSharedRef()
				]
				+SVerticalBox::Slot().AutoHeight()
				[
					HSH_SCROLL.ToSharedRef()
				]
			]
			+SVerticalBox::Slot()
			.AutoHeight().Padding(10,0,10,5)
			[
				SNew(SBorder)
				.VAlign(VAlign_Fill).HAlign(HAlign_Fill)
				.BorderImage(FEditorStyle::GetBrush("Menu.Background"))
				[
					SNew(STextBlock).Margin(FMargin(5,0,0,0))
					.Text(this,&SKMGC_MagicNodeWidget::GetCursorLocation)
					.ColorAndOpacity(FSlateColor(FLinearColor(FColor(225,225,255,225))))
				]
			]
			+SVerticalBox::Slot()
			.AutoHeight().Padding(10,5,10,5)
			[
				SNew(SHorizontalBox)
				.IsEnabled(this,&SKMGC_MagicNodeWidget::HasScript)
				+SHorizontalBox::Slot().AutoWidth().Padding(0,0,5,0)
				[
					SNew(SButton)
					.OnClicked(this,&SKMGC_MagicNodeWidget::OnClickedCompile)
					.ButtonStyle(FEditorStyle::Get(),"FlatButton.DarkGrey")
					.ForegroundColor(FSlateColor::UseForeground())
					.VAlign(VAlign_Center).HAlign(HAlign_Left)
					[
						SNew(SImage)
						.Image(this,&SKMGC_MagicNodeWidget::GetCompilerIcon)
					]
				]
				+SHorizontalBox::Slot().AutoWidth().Padding(0,0,5,0)
				[
					SNew(SButton)
					.OnClicked(this,&SKMGC_MagicNodeWidget::OnClickedReloadScript)
					.ButtonStyle(FEditorStyle::Get(),"FlatButton.DarkGrey")
					.ForegroundColor(FSlateColor::UseForeground())
					.VAlign(VAlign_Center).HAlign(HAlign_Left)
					[
						SNew(SImage)
						.Image(FKMGC_NodeStyle::Get()->GetBrush(TEXT("KMGC.Toolbar.ReloadScript")))
					]
				]
				+SHorizontalBox::Slot().AutoWidth().Padding(0,0,5,0)
				[
					SNew(SButton)
					.OnClicked(this,&SKMGC_MagicNodeWidget::OnClickedSaveScript)
					.ButtonStyle(FEditorStyle::Get(),"FlatButton.DarkGrey")
					.ForegroundColor(FSlateColor::UseForeground())
					.VAlign(VAlign_Center).HAlign(HAlign_Left)
					[
						SNew(SImage)
						.Image(FKMGC_NodeStyle::Get()->GetBrush(TEXT("KMGC.Toolbar.SaveScript")))
					]
				]
			]
		]
	]
	+SHorizontalBox::Slot()
	.HAlign(HAlign_Right).AutoWidth().Padding(FMargin(2.f,0.f,0.f,0.f))
	[
		SNew(SBorder)
		.VAlign(VAlign_Fill).HAlign(HAlign_Fill)
		.BorderImage(FEditorStyle::GetBrush("Graph.Node.Body"))
		[
			SNew(SBox)
			.VAlign(VAlign_Fill).HAlign(HAlign_Fill)
			.MinDesiredWidth(50.f).MinDesiredHeight(250.f)
			.MaxDesiredHeight(1010.f).MaxDesiredWidth(250.f)
			.Visibility(this,&SKMGC_MagicNodeWidget::GetAutoCompleteVisibility)
			[
				SAssignNew(AUTOCOMPLETE_BOX,SScrollBox)
				.OnUserScrolled(this,&SKMGC_MagicNodeWidget::OnAutoCompleteScroll)
				.Orientation(EOrientation::Orient_Vertical)
				.ScrollBarThickness(FVector2D(8.f,8.f))
				+SScrollBox::Slot()
				.VAlign(VAlign_Fill).HAlign(HAlign_Fill)
				[
					SAssignNew(AUTOCOMPLETE,SListView<TSharedPtr<FString>>)
					.OnMouseButtonDoubleClick(this,&SKMGC_MagicNodeWidget::OnClickedAutoCompleteItem)
					.OnSelectionChanged(this,&SKMGC_MagicNodeWidget::OnSelectedAutoCompleteItem)
					.OnGenerateRow(this,&SKMGC_MagicNodeWidget::OnGenerateAutoComplete)
					.ExternalScrollbar(AUTOCOMPLETE_SCROLL)
					.SelectionMode(ESelectionMode::Single)
					.ListItemsSource(&AutoCompleteList)
				]
			]
		]
	]
	];
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void SKMGC_MagicNodeWidget::UpdateGraphNode() {
	SGraphNode::UpdateGraphNode();
}

void SKMGC_MagicNodeWidget::UpdateDatabaseReferences() {
	const auto &_Settings = GetDefault<UKMGC_Settings>();
	//
	TArray<UMGC_SemanticDB*>SemanticDB;
	TArray<UMGC_FunctionDB*>FunctionDB;
	TArray<UMGC_KeywordDB*>KeywordDB;
	TArray<UMGC_ClassDB*>ClassDB;
	//
	for (auto DB : _Settings->KeywordDB.Array()) {
		if (DB.IsValid()) {KeywordDB.Add(DB.LoadSynchronous());}
	}///
	//
	for (auto DB : _Settings->ClassDB.Array()) {
		if (DB.IsValid()) {ClassDB.Add(DB.LoadSynchronous());}
	}///
	//
	for (auto DB : _Settings->FunctionDB.Array()) {
		if (DB.IsValid()) {FunctionDB.Add(DB.LoadSynchronous());}
	}///
	//
	for (auto DB : _Settings->SemanticDB.Array()) {
		if (DB.IsValid()) {SemanticDB.Add(DB.LoadSynchronous());}
	}///
	//
	//
	MARSHALL = FKMGC_TextSyntaxHighlighter::Create(
		FKMGC_TextSyntaxHighlighter::FSyntaxTextStyle(KeywordDB,ClassDB,FunctionDB,SemanticDB)
	);///
	//
	SetLineCountList(GetLineCount());
	//
	if (HasScript()) {
		UKMGC_MagicNode* KNode = (CastChecked<UKMGC_MagicNode>(GraphNode));
		KNode->UpdateDatabaseReferences();
	}///
}

void SKMGC_MagicNodeWidget::UpdateDatabaseSemantics() {
	const auto &_Settings = GetDefault<UKMGC_Settings>();
	//
	for (auto DB : _Settings->SemanticDB.Array()) {
		if (DB.IsValid()) {DB.LoadSynchronous()->UpdateExtensions();}
	}///
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool SKMGC_MagicNodeWidget::IsInteractable() const {
	return IsEnabled();
}

bool SKMGC_MagicNodeWidget::SupportsKeyboardFocus() const {
	return true;
}

bool SKMGC_MagicNodeWidget::HasScript() const {
	return (GetScriptObject()!=nullptr);
}

bool SKMGC_MagicNodeWidget::IsScriptSourceEditable() const {
	if (!HasScript()) {return false;}
	//
	UMagicNodeScript* Script = (CastChecked<UKMGC_MagicNode>(GraphNode))->GetScriptObject();
	//
	return !Script->LockSourceCode;
}

ECheckBoxState SKMGC_MagicNodeWidget::IsSearchSensitive() const {
	return (SensitiveSearch) ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void SKMGC_MagicNodeWidget::SetScriptObject(UObject* New) {
	if (GraphNode==nullptr) {return;}
	//
	UMagicNodeScript* Script = Cast<UMagicNodeScript>(New);
	CastChecked<UKMGC_MagicNode>(GraphNode)->SetScriptObject(Script);
}

void SKMGC_MagicNodeWidget::SetScriptText(const FText &NewText) {
	if (!IsScriptSourceEditable()) {return;}
	//
	UMagicNodeScript* Script = (CastChecked<UKMGC_MagicNode>(GraphNode))->GetScriptObject();
	//
	if (Script->Source.Script != NewText.ToString()) {
		Script->Modify();
		Script->Source.Script = NewText.ToString();
	}///
	//
	(CastChecked<UKMGC_MagicNode>(GraphNode))->SetTooltip(Script->Source.Script);
}

void SKMGC_MagicNodeWidget::SetHeaderText(const FText &NewText) {
	if (!IsScriptSourceEditable()) {return;}
	//
	UMagicNodeScript* Script = (CastChecked<UKMGC_MagicNode>(GraphNode))->GetScriptObject();
	//
	if (Script->Source.Header != NewText.ToString()) {
		Script->Modify();
		Script->Source.Header = NewText.ToString();
	}///
}

void SKMGC_MagicNodeWidget::SetTypesText(const FText &NewText) {
	if (!IsScriptSourceEditable()) {return;}
	//
	UMagicNodeScript* Script = (CastChecked<UKMGC_MagicNode>(GraphNode))->GetScriptObject();
	//
	if (Script->Source.Types != NewText.ToString()) {
		Script->Modify();
		Script->Source.Types = NewText.ToString();
	}///
}

void SKMGC_MagicNodeWidget::SetLineCountList(const int32 Count) {
	LineCount.Empty();
	//
	for (int32 I=1; I<=Count; I++) {
		FString ID = FString::Printf(TEXT("%i"),I);
		LineCount.Add(MakeShareable(new FString(*ID)));
	}///
	//
	if (LINE_COUNTER.IsValid()) {LINE_COUNTER->RequestListRefresh();}
}

void SKMGC_MagicNodeWidget::SetAutoCompleteList(const TArray<FString>&List) {
	AutoCompleteList.Empty();
	//
	for (const FString &S : List) {
		AutoCompleteList.Add(MakeShareable(new FString(*S)));
	}///
	//
	if (AUTOCOMPLETE.IsValid()) {AUTOCOMPLETE->RequestListRefresh();}
}

void SKMGC_MagicNodeWidget::SetIncludesList(const TArray<FString>&List) {
	SourceIncludes.Empty();
	//
	for (const FString &S : List) {
		SourceIncludes.Add(MakeShareable(new FString(*S)));
	}///
	//
	if (INCLUDES_LIST.IsValid()) {INCLUDES_LIST->RequestListRefresh();}
}

void SKMGC_MagicNodeWidget::SetMacrosList(const TArray<FString>&List) {
	SourceMacros.Empty();
	//
	for (const FString &S : List) {
		SourceMacros.Add(MakeShareable(new FString(*S)));
	}///
	//
	if (MACROS_LIST.IsValid()) {MACROS_LIST->RequestListRefresh();}
}

void SKMGC_MagicNodeWidget::SetScriptIncludes(const TArray<FString>&List) {
	if (!IsScriptSourceEditable()) {return;}
	//
	UMagicNodeScript* Script = (CastChecked<UKMGC_MagicNode>(GraphNode))->GetScriptObject();
	Script->Source.Includes = List;
	//
	SetIncludesList(Script->Source.Includes);
}

void SKMGC_MagicNodeWidget::SetScriptMacros(const TArray<FString>&List) {
	if (!IsScriptSourceEditable()) {return;}
	//
	UMagicNodeScript* Script = (CastChecked<UKMGC_MagicNode>(GraphNode))->GetScriptObject();
	Script->Source.Macros = List;
	//
	SetMacrosList(Script->Source.Macros);
}

void SKMGC_MagicNodeWidget::SetErrorMessage(const FString &NewError) {
	ErrorMsg = NewError;
	//
	RefreshErrorInfo();
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

UObject* SKMGC_MagicNodeWidget::GetScriptObject() const {
	if (GraphNode==nullptr) {return nullptr;}
	//
	return (CastChecked<UKMGC_MagicNode>(GraphNode))->GetScriptObject();
}

FText SKMGC_MagicNodeWidget::GetScriptText() const {
	if (!HasScript()) {return FText();}
	//
	UMagicNodeScript* Script = (CastChecked<UKMGC_MagicNode>(GraphNode))->GetScriptObject();
	//
	return FText::FromString(Script->Source.Script);
}

FText SKMGC_MagicNodeWidget::GetHeaderText() const {
	if (!HasScript()) {return FText();}
	//
	UMagicNodeScript* Script = (CastChecked<UKMGC_MagicNode>(GraphNode))->GetScriptObject();
	//
	return FText::FromString(Script->Source.Header);
}

FText SKMGC_MagicNodeWidget::GetTypesText() const {
	if (!HasScript()) {return FText();}
	//
	UMagicNodeScript* Script = (CastChecked<UKMGC_MagicNode>(GraphNode))->GetScriptObject();
	//
	return FText::FromString(Script->Source.Types);
}

FText SKMGC_MagicNodeWidget::GetSearchText() const {
	if (!HEADER_EDITOR.IsValid()||!SCRIPT_EDITOR.IsValid()) {return FText::FromString(TEXT("Search.."));}
	//
	if (Source==ESKMGC_Source::Header) {
		return HEADER_EDITOR->GetSearchText();
	} else if (Source==ESKMGC_Source::Script) {
		return SCRIPT_EDITOR->GetSearchText();
	}///
	//
	return FText();
}

FText SKMGC_MagicNodeWidget::GetReplaceText() const {
	return FText::FromString(ReplaceText);
}

TArray<FString>SKMGC_MagicNodeWidget::GetScriptIncludes() const {
	if (!HasScript()) {return TArray<FString>();}
	//
	UMagicNodeScript* Script = (CastChecked<UKMGC_MagicNode>(GraphNode))->GetScriptObject();
	//
	return Script->Source.Includes;
}

TArray<FString> SKMGC_MagicNodeWidget::GetScriptMacros() const {
	if (!HasScript()) {return TArray<FString>();}
	//
	UMagicNodeScript* Script = (CastChecked<UKMGC_MagicNode>(GraphNode))->GetScriptObject();
	//
	return Script->Source.Macros;
}

FText SKMGC_MagicNodeWidget::GetCursorLocation() const {
	if (!HEADER_EDITOR.IsValid()||!SCRIPT_EDITOR.IsValid()) {return FText();}
	//
	if (Source==ESKMGC_Source::Header) {
		return FText::FromString(FString::Printf(TEXT("Lin: %i  |  Col: %i"),HEADER_EDITOR->GetCursorLocation().GetLineIndex()+1,HEADER_EDITOR->GetCursorLocation().GetOffset()));
	} else if (Source==ESKMGC_Source::Script) {
		return FText::FromString(FString::Printf(TEXT("Lin: %i  |  Col: %i"),SCRIPT_EDITOR->GetCursorLocation().GetLineIndex()+1,SCRIPT_EDITOR->GetCursorLocation().GetOffset()));
	}///
	//
	return FText();
}

EVisibility SKMGC_MagicNodeWidget::GetScriptEditorVisibility() const {
	if (GraphNode==nullptr) {return EVisibility::Visible;}
	//
	if (GraphNode->AdvancedPinDisplay != ENodeAdvancedPins::Hidden) {return EVisibility::Visible;}
	//
	return EVisibility::Collapsed;
}

EVisibility SKMGC_MagicNodeWidget::GetScriptPanelVisibility() const {
	if (GraphNode==nullptr) {return EVisibility::Collapsed;}
	//
	if (Source==ESKMGC_Source::Script) {return EVisibility::Visible;}
	//
	return EVisibility::Collapsed;
}

EVisibility SKMGC_MagicNodeWidget::GetHeaderPanelVisibility() const {
	if (GraphNode==nullptr) {return EVisibility::Collapsed;}
	//
	if (Source==ESKMGC_Source::Header) {return EVisibility::Visible;}
	//
	return EVisibility::Collapsed;
}

EVisibility SKMGC_MagicNodeWidget::GetTypesPanelVisibility() const {
	if (GraphNode==nullptr) {return EVisibility::Collapsed;}
	//
	if (Source==ESKMGC_Source::Types) {return EVisibility::Visible;}
	//
	return EVisibility::Collapsed;
}

EVisibility SKMGC_MagicNodeWidget::GetAutoCompleteVisibility() const {
	if (AutoComplete==EAutoComplete::Active) {return EVisibility::Visible;}
	//
	return EVisibility::Collapsed;
}

EVisibility SKMGC_MagicNodeWidget::GetIncludesPanelVisibility() const {
	if (ViewIncludes && SourceIncludes.Num()>=1) {return EVisibility::Visible;}
	//
	return EVisibility::Collapsed;
}

EVisibility SKMGC_MagicNodeWidget::GetMacrosPanelVisibility() const {
	if (ViewMacros && SourceMacros.Num()>=1) {return EVisibility::Visible;}
	//
	return EVisibility::Collapsed;
}

EVisibility SKMGC_MagicNodeWidget::GetSearchBoxVisibility() const {
	if (!HasScript()) {return EVisibility::Collapsed;}
	//
	return (ViewSearchBox) ? EVisibility::Visible : EVisibility::Collapsed;
}

FSlateColor SKMGC_MagicNodeWidget::GetScriptIconColor() const {
	if (Source!=ESKMGC_Source::Script) {
		return FSlateColor(FLinearColor(0.45f,0.45f,0.45f,0.35f));
	} return FSlateColor(FLinearColor::White);
}

FSlateColor SKMGC_MagicNodeWidget::GetHeaderIconColor() const {
	if (Source!=ESKMGC_Source::Header) {
		return FSlateColor(FLinearColor(0.45f,0.45f,0.45f,0.35f));
	} return FSlateColor(FLinearColor::White);
}

FSlateColor SKMGC_MagicNodeWidget::GetTypesIconColor() const {
	if (Source!=ESKMGC_Source::Types) {
		return FSlateColor(FLinearColor(0.45f,0.45f,0.45f,0.35f));
	} return FSlateColor(FLinearColor::White);
}

int32 SKMGC_MagicNodeWidget::GetLineCount() const {
	TArray<FString>Array;
	int32 Count = 0;
	//
	if (Source==ESKMGC_Source::Header) {
		FString Text = GetHeaderText().ToString();
		Count = Text.ParseIntoArray(Array,TEXT("\n"),false);
	} else if (Source==ESKMGC_Source::Script) {
		FString Text = GetScriptText().ToString();
		Count = Text.ParseIntoArray(Array,TEXT("\n"),false);
	} else if (Source==ESKMGC_Source::Types) {
		FString Text = GetTypesText().ToString();
		Count = Text.ParseIntoArray(Array,TEXT("\n"),false);
	}///
	//
	return Count;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

const FSlateBrush* SKMGC_MagicNodeWidget::GetCompilerIcon() const {
	if (!HasScript()) {return FEditorStyle::GetBrush(TEXT("Kismet.Status.Good"));}
	UPackage* Package = GetScriptObject()->GetOutermost();
	//
	if (Package->IsDirty() && GraphNode->ErrorType >= EMessageSeverity::Info) {
		return FEditorStyle::GetBrush(TEXT("Kismet.Status.Unknown"));
	} else if (GraphNode->ErrorType == EMessageSeverity::Warning) {
		return FEditorStyle::GetBrush(TEXT("Kismet.Status.Warning"));
	} else if (GraphNode->ErrorType <= EMessageSeverity::Error) {
		return FEditorStyle::GetBrush(TEXT("Kismet.Status.Error"));
	} else {return FEditorStyle::GetBrush(TEXT("Kismet.Status.Good"));}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void SKMGC_MagicNodeWidget::OnScriptHorizontalScroll(float Offset) {}
void SKMGC_MagicNodeWidget::OnHeaderHorizontalScroll(float Offset) {}

void SKMGC_MagicNodeWidget::OnVerticalScroll(float Offset) {
	VS_SCROLL->SetState(VS_SCROLL_BOX->GetScrollOffset(),VS_SCROLL_BOX->GetViewOffsetFraction());
}

void SKMGC_MagicNodeWidget::OnInternalVerticalScroll(float Offset) {
	VS_SCROLL_BOX->SetScrollOffset(Offset);
}

void SKMGC_MagicNodeWidget::OnAutoCompleteScroll(float Offset) {
	AUTOCOMPLETE_SCROLL->SetState(AUTOCOMPLETE_BOX->GetScrollOffset(),AUTOCOMPLETE_BOX->GetViewOffsetFraction());
}

void SKMGC_MagicNodeWidget::OnInternalAutoCompleteScroll(float Offset) {
	AUTOCOMPLETE_BOX->SetScrollOffset(Offset);
}

void SKMGC_MagicNodeWidget::OnScriptTextChanged(const FText &InText, ETextCommit::Type CommitType) {
	if (!IsScriptSourceEditable()) {return;}
	//
	//
	TArray<FString>Lines;
	InText.ToString().ParseIntoArrayLines(Lines,false);
	//
	if (AutoComplete==EAutoComplete::Active) {
		FString Subject = SCRIPT_EDITOR->ParseAutoCompleteWord(Lines,true);
		if (AutoCompleteList.Num()>=1) {OnAdvanceAutoComplete(Subject);}
		else {SCRIPT_EDITOR->AutoCompleteSubject(Subject);}
	} else {
		FString Subject = SCRIPT_EDITOR->ParseAutoCompleteWord(Lines);
		if (SCRIPT_EDITOR->IsAutoComplete(Subject)) {
			SCRIPT_EDITOR->AutoCompleteSubject(Subject);
		}///
	}///
	//
	//
	SetScriptText(InText);
	SetLineCountList(GetLineCount());
}

void SKMGC_MagicNodeWidget::OnHeaderTextChanged(const FText &InText, ETextCommit::Type CommitType) {
	if (!IsScriptSourceEditable()) {return;}
	//
	//
	TArray<FString>Lines;
	InText.ToString().ParseIntoArrayLines(Lines,false);
	//
	if (AutoComplete==EAutoComplete::Active) {
		FString Subject = HEADER_EDITOR->ParseAutoCompleteWord(Lines,true);
		if (AutoCompleteList.Num()>=1) {OnAdvanceAutoComplete(Subject);}
		else {HEADER_EDITOR->AutoCompleteSubject(Subject);}
	} else {
		FString Subject = HEADER_EDITOR->ParseAutoCompleteWord(Lines);
		if (HEADER_EDITOR->IsAutoComplete(Subject)) {
			HEADER_EDITOR->AutoCompleteSubject(Subject);
		}///
	}///
	//
	//
	SetHeaderText(InText);
	SetLineCountList(GetLineCount());
}

void SKMGC_MagicNodeWidget::OnTypesTextChanged(const FText &InText, ETextCommit::Type CommitType) {
	if (!IsScriptSourceEditable()) {return;}
	//
	SetTypesText(InText);
}

void SKMGC_MagicNodeWidget::OnSearchTextChanged(const FText &InText, ETextCommit::Type CommitType) {
	SearchText = InText.ToString();
	//
	OnClickedSearchGlass();
}

void SKMGC_MagicNodeWidget::OnScriptTextComitted(const FText &NewText, ETextCommit::Type CommitInfo) {
	if (!IsScriptSourceEditable()) {return;}
	//
	SetScriptText(NewText);
	SetLineCountList(GetLineCount());
}

void SKMGC_MagicNodeWidget::OnHeaderTextComitted(const FText &NewText, ETextCommit::Type CommitInfo) {
	if (!IsScriptSourceEditable()) {return;}
	//
	SetHeaderText(NewText);
	SetLineCountList(GetLineCount());
}

void SKMGC_MagicNodeWidget::OnTypesTextComitted(const FText &NewText, ETextCommit::Type CommitInfo) {
	if (!IsScriptSourceEditable()) {return;}
	//
	SetTypesText(NewText);
}

void SKMGC_MagicNodeWidget::OnIncludeTextComitted(const FText &NewText, ETextCommit::Type CommitInfo, const FString Old) {
	if (!IsScriptSourceEditable()) {return;}
	//
	FString Cleansed = NewText.ToString();
	Cleansed.Split(TEXT("\""),&Cleansed,nullptr,ESearchCase::IgnoreCase,ESearchDir::FromEnd);
	Cleansed.Split(TEXT("\""),nullptr,&Cleansed,ESearchCase::IgnoreCase,ESearchDir::FromEnd);
	UMagicNodeScript* Script = (CastChecked<UKMGC_MagicNode>(GraphNode))->GetScriptObject();
	//
	Script->Modify();
	if (Script->Source.Includes.Contains(Old)) {Script->Source.Includes.Remove(Old);}
	if (!Script->Source.Includes.Contains(Cleansed)) {Script->Source.Includes.Add(Cleansed);}
	//
	SetIncludesList(Script->Source.Includes);
}

void SKMGC_MagicNodeWidget::OnMacroTextComitted(const FText &NewText, ETextCommit::Type CommitInfo, const FString Old) {
	if (!IsScriptSourceEditable()) {return;}
	//
	UMagicNodeScript* Script = (CastChecked<UKMGC_MagicNode>(GraphNode))->GetScriptObject();
	//
	Script->Modify(); const FString NewMacro = NewText.ToString();
	if (Script->Source.Macros.Contains(Old)) {Script->Source.Macros.Remove(Old);}
	if (!Script->Source.Macros.Contains(NewMacro)) {Script->Source.Macros.Add(NewMacro);}
	//
	SetMacrosList(Script->Source.Macros);
}

void SKMGC_MagicNodeWidget::OnSearchTextComitted(const FText &NewText, ETextCommit::Type CommitInfo) {
	SearchText.Empty(); SearchText.Append(NewText.ToString());
	//
	OnClickedSearchGlass();
}

void SKMGC_MagicNodeWidget::OnReplaceTextComitted(const FText &NewText, ETextCommit::Type CommitInfo) {
	ReplaceText.Empty(); ReplaceText.Append(NewText.ToString());
}

void SKMGC_MagicNodeWidget::OnAutoComplete(const TArray<FString>&Results) {
	if (!IsScriptSourceEditable()) {return;}
	//
	if (Results.Num()<=0) {
		SetAutoCompleteList(Results);
		AutoComplete=EAutoComplete::Off;
	return;}
	//
	TArray<FString>Results_PROP;
	TArray<FString>Results_FUN;
	//
	for (const FString &Item : Results) {
		if (Item.Contains(TEXT("VAR|"))) {Results_PROP.Add(Item);} else
		if (Item.Contains(TEXT("FUN|"))) {Results_FUN.Add(Item);}
	}///
	//
	Results_PROP.Append(Results_FUN);
	SetAutoCompleteList(Results_PROP);
	//
	if (Results_PROP.Num()>=1||Results_FUN.Num()>=1) {AutoComplete=EAutoComplete::Active;} else {AutoComplete=EAutoComplete::Off;}
}

void SKMGC_MagicNodeWidget::OnAdvanceAutoComplete(const FString &Search) {
	if (!IsScriptSourceEditable()) {return;}
	//
	if (AutoComplete==EAutoComplete::Off) {return;}
	if (AutoCompleteList.Num()==0) {return;}
	//
	TArray<FString>Found;
	for (const TSharedPtr<FString>&Item : AutoCompleteList) {
		if (!Item.IsValid()) {continue;}
		//
		if (Item->Contains(Search)) {Found.Add(*Item.Get());}
	}///
	//
	if (Found.Num()>=1) {SetAutoCompleteList(Found);}
}

void SKMGC_MagicNodeWidget::OnInvokedSearch() {
	ReplaceText = TEXT("Replace..");
	ViewSearchBox = !ViewSearchBox;
	//
	if (ViewSearchBox) {
		FSlateApplication::Get().SetKeyboardFocus(SEARCH_TEXT.ToSharedRef());
		if (HEADER_EDITOR.IsValid()) {HEADER_EDITOR->SetSearchText(FText::FromString(HEADER_EDITOR->GetUnderCursor()));}
		if (SCRIPT_EDITOR.IsValid()) {SCRIPT_EDITOR->SetSearchText(FText::FromString(SCRIPT_EDITOR->GetUnderCursor()));}
	}///
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

FReply SKMGC_MagicNodeWidget::OnClickedScriptIcon() {
	if (!HasScript()) {return FReply::Unhandled();}
	//
	Source = ESKMGC_Source::Script;
	SetLineCountList(GetLineCount());
	//
	HSS_SCROLL->SetVisibility(EVisibility::Visible);
	HSH_SCROLL->SetVisibility(EVisibility::Collapsed);
	//
	return FReply::Handled();
}

FReply SKMGC_MagicNodeWidget::OnClickedHeaderIcon() {
	if (!HasScript()) {return FReply::Unhandled();}
	//
	Source = ESKMGC_Source::Header;
	SetLineCountList(GetLineCount());
	//
	HSS_SCROLL->SetVisibility(EVisibility::Collapsed);
	HSH_SCROLL->SetVisibility(EVisibility::Visible);
	//
	return FReply::Handled();
}

FReply SKMGC_MagicNodeWidget::OnClickedTypesIcon() {
	if (!HasScript()) {return FReply::Unhandled();}
	//
	Source = ESKMGC_Source::Types;
	SetLineCountList(GetLineCount());
	//
	HSS_SCROLL->SetVisibility(EVisibility::Collapsed);
	HSH_SCROLL->SetVisibility(EVisibility::Collapsed);
	//
	return FReply::Handled();
}

FReply SKMGC_MagicNodeWidget::OnClickedViewIncludes() {
	if (!HasScript()) {return FReply::Unhandled();}
	//
	SetIncludesList(GetScriptIncludes());
	ViewIncludes = !ViewIncludes;
	//
	return FReply::Handled();
}

FReply SKMGC_MagicNodeWidget::OnClickedViewMacros() {
	if (!HasScript()) {return FReply::Unhandled();}
	//
	SetMacrosList(GetScriptMacros());
	ViewMacros = !ViewMacros;
	//
	return FReply::Handled();
}

FReply SKMGC_MagicNodeWidget::OnClickedAddInclude() {
	if (!IsScriptSourceEditable()) {return FReply::Unhandled();}
	//
	UMagicNodeScript* Script = (CastChecked<UKMGC_MagicNode>(GraphNode))->GetScriptObject();
	if (!Script->Source.Includes.Contains(FString(TEXT("Runtime/")))) {
		Script->Modify(); Script->Source.Includes.Add(FString(TEXT("Runtime/")));
	}///
	//
	SetIncludesList(Script->Source.Includes);
	ViewIncludes = true;
	//
	return FReply::Handled();
}

FReply SKMGC_MagicNodeWidget::OnClickedAddMacro() {
	if (!IsScriptSourceEditable()) {return FReply::Unhandled();}
	//
	UMagicNodeScript* Script = (CastChecked<UKMGC_MagicNode>(GraphNode))->GetScriptObject();
	if (!Script->Source.Macros.Contains(FString(TEXT("DECLARE_DELEGATE(FMyScriptEvent);")))) {
		Script->Modify(); Script->Source.Macros.Add(FString(TEXT("DECLARE_DELEGATE(FMyScriptEvent);")));
	}///
	//
	SetMacrosList(Script->Source.Macros);
	ViewMacros = true;
	//
	return FReply::Handled();
}

FReply SKMGC_MagicNodeWidget::OnClickedSearchGlass() {
	const ESearchCase::Type Case = (SensitiveSearch) ? ESearchCase::CaseSensitive : ESearchCase::IgnoreCase;
	if (!HasScript()) {return FReply::Unhandled();}
	//
	if (ReplaceText==TEXT("Replace..")) {ReplaceText=TEXT("");}
	//
	if (Source==ESKMGC_Source::Header && HEADER_EDITOR.IsValid()) {
		FSlateApplication::Get().SetKeyboardFocus(HEADER_EDITOR.ToSharedRef());
		HEADER_EDITOR->BeginSearch(FText::FromString(SearchText),Case,true);
	} else if (Source==ESKMGC_Source::Script && SCRIPT_EDITOR.IsValid()) {
		FSlateApplication::Get().SetKeyboardFocus(SCRIPT_EDITOR.ToSharedRef());
		SCRIPT_EDITOR->BeginSearch(FText::FromString(SearchText),Case,false);
	}///
	//
	return FReply::Handled();
}

FReply SKMGC_MagicNodeWidget::OnClickedRemoveInclude(const FString Item) {
	if (!IsScriptSourceEditable()) {return FReply::Unhandled();}
	//
	UMagicNodeScript* Script = (CastChecked<UKMGC_MagicNode>(GraphNode))->GetScriptObject();
	if (Script->Source.Includes.Contains(Item)) {
		Script->Modify();
		Script->Source.Includes.Remove(Item);
		SetIncludesList(Script->Source.Includes);
	}///
	//
	return FReply::Handled();
}

FReply SKMGC_MagicNodeWidget::OnClickedRemoveMacro(const FString Item) {
	if (!IsScriptSourceEditable()) {return FReply::Unhandled();}
	//
	UMagicNodeScript* Script = (CastChecked<UKMGC_MagicNode>(GraphNode))->GetScriptObject();
	if (Script->Source.Macros.Contains(Item)) {
		Script->Modify();
		Script->Source.Macros.Remove(Item);
		SetMacrosList(Script->Source.Macros);
	}///
	//
	return FReply::Handled();
}

void SKMGC_MagicNodeWidget::OnSearchSensitiveChanged(ECheckBoxState NewState) {
	SensitiveSearch = (NewState==ECheckBoxState::Checked);
	//
	if (ReplaceText==TEXT("Replace..")) {ReplaceText=TEXT("");}
}

FReply SKMGC_MagicNodeWidget::OnClickedReplaceSearch() {
	if (!HEADER_EDITOR.IsValid()||!SCRIPT_EDITOR.IsValid()) {return FReply::Unhandled();}
	if (!IsScriptSourceEditable()) {return FReply::Unhandled();}
	//
	if (Source==ESKMGC_Source::Header) {
		FSlateApplication::Get().SetKeyboardFocus(HEADER_EDITOR.ToSharedRef());
		//
		HEADER_EDITOR->AdvanceSearch();
		HEADER_EDITOR->DeleteSelectedText();
		HEADER_EDITOR->InsertTextAtCursor(ReplaceText);
	} else if (Source==ESKMGC_Source::Script) {
		FSlateApplication::Get().SetKeyboardFocus(SCRIPT_EDITOR.ToSharedRef());
		//
		SCRIPT_EDITOR->AdvanceSearch();
		SCRIPT_EDITOR->DeleteSelectedText();
		SCRIPT_EDITOR->InsertTextAtCursor(ReplaceText);
	}///
	//
	return FReply::Handled();
}

FReply SKMGC_MagicNodeWidget::OnClickedSaveScript() {
	if (!IsScriptSourceEditable()) {return FReply::Unhandled();}
	//
	UPackage* Package = GetScriptObject()->GetOutermost();
	TArray<UPackage*>PackagesToSave;
	PackagesToSave.Add(Package);
	//
	if (Package->IsDirty()) {
		FEditorFileUtils::PromptForCheckoutAndSave(PackagesToSave,true,false);
	}///
	//
	return FReply::Handled();
}

FReply SKMGC_MagicNodeWidget::OnClickedReloadScript() {
	if (!IsScriptSourceEditable()) {return FReply::Unhandled();}
	//
	CastChecked<UKMGC_MagicNode>(GraphNode)->ReloadRuntimeScript();
	//
	return FReply::Handled();
}

FReply SKMGC_MagicNodeWidget::OnClickedCompile() {
	if (!IsScriptSourceEditable()) {return FReply::Unhandled();}
	//
	CastChecked<UKMGC_MagicNode>(GraphNode)->CompileScript();
	//
	return FReply::Handled();
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void SKMGC_MagicNodeWidget::OnSelectedLineCounterItem(TSharedPtr<FString>Item, ESelectInfo::Type SelectInfo) {
	if (!Item.IsValid()) {return;}
	//
	const int32 LineID = FCString::Atoi(**Item.Get());
	//
	if (Source==ESKMGC_Source::Script) {
		FSlateApplication::Get().SetKeyboardFocus(SCRIPT_EDITOR.ToSharedRef());
		SCRIPT_EDITOR->GoToLineColumn(LineID-1,0);
		SCRIPT_EDITOR->SelectLine();
	}///
	//
	if (Source==ESKMGC_Source::Header) {
		FSlateApplication::Get().SetKeyboardFocus(HEADER_EDITOR.ToSharedRef());
		HEADER_EDITOR->GoToLineColumn(LineID-1,0);
		HEADER_EDITOR->SelectLine();
	}///
	//
	LINE_COUNTER->SetItemSelection(Item,false);
}

void SKMGC_MagicNodeWidget::OnSelectedAutoCompleteItem(TSharedPtr<FString>Item, ESelectInfo::Type SelectInfo) {
	if (!HEADER_EDITOR.IsValid()||!SCRIPT_EDITOR.IsValid()) {return;}
	if (!IsScriptSourceEditable()) {return;}
	if (!Item.IsValid()) {return;}
	//
	if (AutoComplete==EAutoComplete::Off) {return;}
	//
	//
	UKMGC_MagicNode* KNode = (CastChecked<UKMGC_MagicNode>(GraphNode));
	UMagicNodeScript* Script = KNode->GetScriptObject();
	//
	FString Subject = FString(TEXT("U"))+Script->GetRuntimeScriptClass()->GetName();
	if (Source==ESKMGC_Source::Header) {Subject=HEADER_EDITOR->GetAutoCompleteSubject();}
	if (Source==ESKMGC_Source::Script) {Subject=SCRIPT_EDITOR->GetAutoCompleteSubject();}
	//
	FString Keyword = (**Item.Get());
	Keyword.Split(TEXT("|"),nullptr,&Keyword,ESearchCase::IgnoreCase,ESearchDir::FromEnd);
	//
	const FFunctionDefinition &FunInfo = IKMGC_ScriptParser::GetFunctionInfo(Subject,Keyword);
	if (FunInfo==IKMGC_ScriptParser::NOFunctionInfo) {return;}
	//
	FString Info = Keyword+FString(TEXT(" ::\n\n"));
	if (!FunInfo.Hint.IsEmpty()) {Info+=(FunInfo.Hint+FString(TEXT(" \n\n")));}
	if (!FunInfo.TypeToString().Contains("?")) {Info+=(FString(TEXT("Type: \t"))+FunInfo.TypeToString()+FString(TEXT("\n")));}
	if (!FunInfo.AccessToString().Contains("?")) {Info+=(FString(TEXT("Access: \t"))+FunInfo.AccessToString()+FString(TEXT("\n\n")));}
	//
	if (!FunInfo.ReturnType.IsEmpty()) {Info+=(FString(TEXT("Return: \t"))+FunInfo.ReturnType+FString(TEXT("\n\n")));}
	//
	if (FunInfo.Inputs.Num()>=1) {
		Info+=(FString(TEXT("Inputs:\n")));
		for (const FString &Input : FunInfo.Inputs) {
			Info += (FString(TEXT("\t"))+Input+FString(TEXT("\n")));
		} Info += FString(TEXT("\n"));
	}///
	//
	if (FunInfo.Outputs.Num()>=1) {
		Info+=(FString(TEXT("Outputs:\n")));
		for (const FString &Output : FunInfo.Outputs) {
			Info += (FString(TEXT("\t"))+Output+FString(TEXT("\n")));
		} Info += FString(TEXT("\n"));
	}///
	//
	if (Info.Contains(TEXT("Return"))) {Info+=FString(TEXT("\n"));}
	if (!FunInfo.Tooltip.IsEmpty()) {Info+=(FunInfo.Tooltip+FString(TEXT("\n")));}
	//
	//
	KNode->SetTooltip(Info);
}

void SKMGC_MagicNodeWidget::OnClickedAutoCompleteItem(TSharedPtr<FString>Item) {
	if (!IsScriptSourceEditable()) {return;}
	//
	AutoComplete = EAutoComplete::Off;
	//
	if (AUTOCOMPLETE.IsValid()&&SCRIPT_EDITOR.IsValid()&&(Source==ESKMGC_Source::Script)) {
		FSlateApplication::Get().SetKeyboardFocus(SCRIPT_EDITOR.ToSharedRef());
		//
		if (Item.IsValid()) {
			FString Result;
			Item.Get()->Split(TEXT("|"),nullptr,&Result,ESearchCase::IgnoreCase,ESearchDir::FromEnd);
			//
			Result.ReplaceInline(*SCRIPT_EDITOR->GetUnderCursor(),TEXT(""));
			//
			if (Item.Get()->Contains(TEXT("FUN|"))) {
				SCRIPT_EDITOR->InsertTextAtCursor(Result+TEXT("()"));
			} else {SCRIPT_EDITOR->InsertTextAtCursor(Result);}
		}///
	} else if (AUTOCOMPLETE.IsValid()&&HEADER_EDITOR.IsValid()&&(Source==ESKMGC_Source::Header)) {
		FSlateApplication::Get().SetKeyboardFocus(HEADER_EDITOR.ToSharedRef());
		//
		if (Item.IsValid()) {
			FString Result;
			Item.Get()->Split(TEXT("|"),nullptr,&Result,ESearchCase::IgnoreCase,ESearchDir::FromEnd);
			//
			Result.ReplaceInline(*HEADER_EDITOR->GetUnderCursor(),TEXT(""));
			//
			if (Item.Get()->Contains(TEXT("FUN|"))) {
				HEADER_EDITOR->InsertTextAtCursor(Result+TEXT("()"));
			} else {HEADER_EDITOR->InsertTextAtCursor(Result);}
		}///
	}///
	//
	if (Item.IsValid()) {AUTOCOMPLETE->SetItemSelection(Item,false);}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

TSharedRef<ITableRow>SKMGC_MagicNodeWidget::OnGenerateLineCounter(TSharedPtr<FString>Item, const TSharedRef<STableViewBase>&OwnerTable) {
	return
	SNew(SComboRow<TSharedRef<FString>>,OwnerTable)
	[
		SNew(SBorder)
		.BorderImage(FEditorStyle::GetBrush("Graph.Node.Body"))
		.ColorAndOpacity(FLinearColor(1.f,1.f,1.f,1.f))
		.Padding(FMargin(5.f,0.f,5.f,0.f))
		[
			SNew(STextBlock)
			.Text(FText::FromString(*Item.Get()))
			.ColorAndOpacity(FSlateColor(FLinearColor(FColor(75,185,245,225))))
			.Font(FKMGC_NodeStyle::Get().Get()->GetWidgetStyle<FTextBlockStyle>("KMGC.CodeBlockStyle").Font)
		]
	];//
}

TSharedRef<ITableRow>SKMGC_MagicNodeWidget::OnGenerateAutoComplete(TSharedPtr<FString>Item, const TSharedRef<STableViewBase>&OwnerTable) {
	if (Item.IsValid()&&(Item.Get()->Contains(TEXT("FUN|")))) {
		FString Caption;
		//
		Item.Get()->Split(TEXT("|"),nullptr,&Caption,ESearchCase::IgnoreCase,ESearchDir::FromEnd);
		//
		uint8 R = 155; uint8 G = 155; uint8 B = 155;
		if (Item.Get()->Contains(TEXT("PB|"))) {R=255, G=125, B=5;}
		if (Item.Get()->Contains(TEXT("PT|"))) {R=55, G=175, B=255;}
		if (Item.Get()->Contains(TEXT("PV|"))) {R=25, G=125, B=225;}
		if (Item.Get()->Contains(TEXT("ST|"))) {R=125, G=125, B=125;}
		FSlateColor Color = FSlateColor(FLinearColor(FColor(R,G,B,255)));
		//
		return
		SNew(SComboRow<TSharedRef<FString>>,OwnerTable)
		[
			SNew(SHorizontalBox)
			+SHorizontalBox::Slot()
			.AutoWidth().VAlign(VAlign_Center).Padding(4,0,4,0)
			[
				SNew(SImage)
				.ColorAndOpacity(Color)
				.Image(FEditorStyle::GetBrush(TEXT("Kismet.AllClasses.FunctionIcon")))
			]
			+SHorizontalBox::Slot()
			.VAlign(VAlign_Center)
			[
				SNew(SBorder)
				.ColorAndOpacity(FLinearColor(1.f,1.f,1.f,1.f))
				.BorderImage(FEditorStyle::GetBrush("Graph.Node.Body"))
				[
					SNew(STextBlock)
					.ColorAndOpacity(Color)
					.Margin(FMargin(2.f,2.f,2.f,2.f))
					.Text(FText::FromString(Caption))
					.Font(FKMGC_NodeStyle::Get().Get()->GetWidgetStyle<FTextBlockStyle>("KMGC.SourceBlockStyle").Font)
				]
			]
		];//
	} else if (Item.IsValid()&&(Item.Get()->Contains(TEXT("VAR|")))) {
		FString Caption;
		//
		Item.Get()->Split(TEXT("|"),nullptr,&Caption,ESearchCase::IgnoreCase,ESearchDir::FromEnd);
		//
		return
		SNew(SComboRow<TSharedRef<FString>>,OwnerTable)
		[
			SNew(SHorizontalBox)
			+SHorizontalBox::Slot()
			.AutoWidth().VAlign(VAlign_Center).Padding(4,0,4,0)
			[
				SNew(SImage)
				.Image(IKMGC_ScriptParser::GetTypeIcon(*Item.Get()))
				.ColorAndOpacity(IKMGC_ScriptParser::GetTypeColor(*Item.Get()))
			]
			+SHorizontalBox::Slot()
			.VAlign(VAlign_Center)
			[
				SNew(SBorder)
				.ColorAndOpacity(FLinearColor(1.f,1.f,1.f,1.f))
				.BorderImage(FEditorStyle::GetBrush("Graph.Node.Body"))
				[
					SNew(STextBlock)
					.Margin(FMargin(2.f,2.f,2.f,2.f))
					.Text(FText::FromString(Caption))
					.ColorAndOpacity(FSlateColor(FLinearColor(FColor(255,235,195,255))))
					.Font(FKMGC_NodeStyle::Get().Get()->GetWidgetStyle<FTextBlockStyle>("KMGC.SourceBlockStyle").Font)
				]
			]
		];//
	}///
	//
	return SNew(SComboRow<TSharedRef<FString>>,OwnerTable);
}

TSharedRef<ITableRow>SKMGC_MagicNodeWidget::OnGenerateIncludes(TSharedPtr<FString>Item, const TSharedRef<STableViewBase>&OwnerTable) {
	return
	SNew(SComboRow<TSharedRef<FString>>,OwnerTable)
	[
		SNew(SHorizontalBox)
		+SHorizontalBox::Slot().AutoWidth()
		.VAlign(VAlign_Center).HAlign(HAlign_Left)
		[
			SNew(SBorder)
			.VAlign(VAlign_Fill).HAlign(HAlign_Fill)
			.BorderImage(FEditorStyle::GetBrush("Graph.Node.Body"))
			[
				SNew(SButton)
				.OnClicked(this,&SKMGC_MagicNodeWidget::OnClickedRemoveInclude,(*Item.Get()))
				.ButtonStyle(FEditorStyle::Get(),"NoBorder")
				.VAlign(VAlign_Center).HAlign(HAlign_Center)
				[
					SNew(SImage)
					.Image(FEditorStyle::GetBrush("Cross"))
				]
			]
		]
		+SHorizontalBox::Slot()
		.VAlign(VAlign_Center).HAlign(HAlign_Fill)
		[
			SNew(SEditableTextBox)
			.OnTextCommitted(this,&SKMGC_MagicNodeWidget::OnIncludeTextComitted,(*Item.Get()))
			.Text(FText::FromString(*Item.Get()))
		]
	];//
}

TSharedRef<ITableRow> SKMGC_MagicNodeWidget::OnGenerateMacros(TSharedPtr<FString>Item, const TSharedRef<STableViewBase>&OwnerTable) {
	return
	SNew(SComboRow<TSharedRef<FString>>,OwnerTable)
	[
		SNew(SHorizontalBox)
		+SHorizontalBox::Slot().AutoWidth()
		.VAlign(VAlign_Center).HAlign(HAlign_Left)
		[
			SNew(SBorder)
			.VAlign(VAlign_Fill).HAlign(HAlign_Fill)
			.BorderImage(FEditorStyle::GetBrush("Graph.Node.Body"))
			[
				SNew(SButton)
				.OnClicked(this,&SKMGC_MagicNodeWidget::OnClickedRemoveMacro,(*Item.Get()))
				.ButtonStyle(FEditorStyle::Get(),"NoBorder")
				.VAlign(VAlign_Center).HAlign(HAlign_Center)
				[
					SNew(SImage)
					.Image(FEditorStyle::GetBrush("Cross"))
				]
			]
		]
		+SHorizontalBox::Slot()
		.VAlign(VAlign_Center).HAlign(HAlign_Fill)
		[
			SNew(SEditableTextBox)
			.OnTextCommitted(this,&SKMGC_MagicNodeWidget::OnMacroTextComitted,(*Item.Get()))
			.Text(FText::FromString(*Item.Get()))
		]
	];//
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

FReply SKMGC_MagicNodeWidget::OnKeyDown(const FGeometry &MyGeometry, const FKeyEvent &KeyEvent) {
	if (!IsEnabled()||!IsScriptSourceEditable()) {return FReply::Unhandled();}
	//
	if ((KeyEvent.GetKey()==EKeys::Enter)&&(AutoComplete==EAutoComplete::Off)) {return FReply::Handled();}
	if (KeyEvent.GetKey()==EKeys::S && (KeyEvent.IsControlDown()||KeyEvent.IsCommandDown())) {
		OnClickedSaveScript(); return FReply::Handled();
	}///
	//
	if ((KeyEvent.GetKey()==EKeys::Enter)&&(AutoComplete==EAutoComplete::Active)) {
		TArray<TSharedPtr<FString>>Items = AUTOCOMPLETE->GetSelectedItems();
		if ((Items.Num()>=1)&&(Items[0].IsValid())) {
			OnClickedAutoCompleteItem(Items[0]);
		} return FReply::Handled();
	}///
	//
	if ((KeyEvent.GetKey()==EKeys::Escape)&&(AutoComplete==EAutoComplete::Active)) {
		AutoComplete = EAutoComplete::Off;
		AutoCompleteList.Empty();
		//
		return FReply::Handled();
	}///
	//
	if ((KeyEvent.GetKey()==EKeys::SpaceBar)&&(KeyEvent.IsControlDown()||KeyEvent.IsCommandDown())&&(AutoComplete==EAutoComplete::Off)) {
		UKMGC_MagicNode* KNode = (CastChecked<UKMGC_MagicNode>(GraphNode));
		UMagicNodeScript* Script = KNode->GetScriptObject();
		//
		if (Script->RefreshRuntimeScriptClass()) {
			FString Subject = FString(TEXT("U"))+Script->GetRuntimeScriptClass()->GetName();
			HEADER_EDITOR->SetAutoCompleteSubject(Subject);
		}///
		//
		return FReply::Handled();
	}///
	//
	return SGraphNode::OnKeyDown(MyGeometry,KeyEvent);
}

FReply SKMGC_MagicNodeWidget::OnMouseMove(const FGeometry &MyGeometry, const FPointerEvent &MouseEvent) {
	const auto &_Settings = GetDefault<UKMGC_Settings>();
	//
	if (!HEADER_EDITOR.IsValid()||!SCRIPT_EDITOR.IsValid()) {return SGraphNode::OnMouseMove(MyGeometry,MouseEvent);}
	if (!IsScriptSourceEditable()||!IsEnabled()) {return SGraphNode::OnMouseMove(MyGeometry,MouseEvent);}
	if (GraphNode==nullptr) {return SGraphNode::OnMouseMove(MyGeometry,MouseEvent);}
	//
	UKMGC_MagicNode* KNode = (CastChecked<UKMGC_MagicNode>(GraphNode));
	UMagicNodeScript* Script = KNode->GetScriptObject();
	//
	if ((HasKeyboardFocus()||HasFocusedDescendants())&&(HEADER_EDITOR->GetCursorLocation().GetOffset()>=1||SCRIPT_EDITOR->GetCursorLocation().GetOffset()>=1)&&(GraphNode->AdvancedPinDisplay!=ENodeAdvancedPins::Hidden)) {
		FString Keyword;
		//
		if (Source==ESKMGC_Source::Header) {
			HEADER_EDITOR->GetSelectedText().ToString().Split(TEXT("\n"),&Keyword,nullptr);
			Keyword.Split(TEXT(" "),&Keyword,nullptr);
			//
			if (Keyword.TrimStart().IsEmpty()) {Keyword=HEADER_EDITOR->GetUnderCursor();}
		} else if (Source==ESKMGC_Source::Script) {
			SCRIPT_EDITOR->GetSelectedText().ToString().Split(TEXT("\n"),&Keyword,nullptr);
			Keyword.Split(TEXT(" "),&Keyword,nullptr);
			//
			if (Keyword.TrimStart().IsEmpty()) {Keyword=SCRIPT_EDITOR->GetUnderCursor();}
		}///
		//
		if (Keyword.TrimStart().IsEmpty()&&(KNode->GetTooltipText().ToString()!=Script->Source.Script)) {
			if (Source==ESKMGC_Source::Header) {KNode->SetTooltip(Script->Source.Script);}
			if (Source==ESKMGC_Source::Script) {KNode->SetTooltip(Script->Source.Header);}
		} else if (HintTimer >= 10.f && (!LastHint.Equals(Keyword))) {
			LastHint = Keyword; HintTimer = 0.f;
			//
			const FKeywordDefinition &KeyInfo = IKMGC_ScriptParser::GetKeywordInfo(Keyword);
			//
			if (!KeyInfo.Info.IsEmpty() && _Settings->ShowKeywordHints) {
				KNode->SetTooltip(Keyword+FString(TEXT(" ::\n\n"))+KeyInfo.Info);
			} else if (_Settings->ShowKeywordHints) {
				const FClassDefinition &ClassInfo = IKMGC_ScriptParser::GetClassPointerInfo(Keyword);
				//
				bool Nill=true;
				FString Info = Keyword+FString(TEXT(" ::\n\n"));
				//
				if (!ClassInfo.Hint.IsEmpty()) {Info+=(ClassInfo.Hint+FString(TEXT("\n\n"))); Nill=false;}
				if (!ClassInfo.Tooltip.IsEmpty()) {Info+=(ClassInfo.Tooltip+FString(TEXT("\n"))); Nill=false;}
				if (!ClassInfo.ParentClass.IsEmpty()) {Info+=(FString(TEXT("Parent Class: \t"))+ClassInfo.ParentClass+FString(TEXT("\n\n"))); Nill=false;}
				//
				if (!Nill) {KNode->SetTooltip(Info);} else {
					const FPropertyDefinition &PropInfo = IKMGC_ScriptParser::GetPropertyInfo(Script->GetRuntimeScriptClass(),Keyword);
					//
					if (!PropInfo.Hint.IsEmpty()) {Info+=(PropInfo.Hint+FString(TEXT("\n\n"))); Nill=false;}
					if (!PropInfo.TypeToString().Contains("?")) {Info+=(FString(TEXT("Type: \t"))+PropInfo.TypeToString()+FString(TEXT("\n"))); Nill=false;}
					if (!PropInfo.AccessToString().Contains("?")) {Info+=(FString(TEXT("Access: \t"))+PropInfo.AccessToString()+FString(TEXT("\n\n"))); Nill=false;}
					if (!PropInfo.Tooltip.IsEmpty()) {Info+=(PropInfo.Tooltip+FString(TEXT("\n"))); Nill=false;}
					//
					if (!Nill) {KNode->SetTooltip(Info);} else {
						const FFunctionDefinition &FunInfo = IKMGC_ScriptParser::GetFunctionInfo(Script->GetRuntimeScriptClass(),Keyword);
						//
						if (!FunInfo.Hint.IsEmpty()) {Info+=(FunInfo.Hint+FString(TEXT("\n\n"))); Nill=false;}
						if (!FunInfo.TypeToString().Contains("?")) {Info+=(FString(TEXT("Type: \t"))+FunInfo.TypeToString()+FString(TEXT("\n"))); Nill=false;}
						if (!FunInfo.AccessToString().Contains("?")) {Info+=(FString(TEXT("Access: \t"))+FunInfo.AccessToString()+FString(TEXT("\n\n"))); Nill=false;}
						//
						if (!FunInfo.ReturnType.IsEmpty()) {Info+=(FString(TEXT("Return: \t"))+FunInfo.ReturnType+FString(TEXT("\n\n"))); Nill=false;}
						if (FunInfo.Inputs.Num()>=1) {
							Info+=(FString(TEXT("Inputs:\n"))); Nill=false;
							for (const FString &Input : FunInfo.Inputs) {
								Info += (FString(TEXT("\t"))+Input+FString(TEXT("\n")));
							} Info += FString(TEXT("\n"));
						}///
						//
						if (FunInfo.Outputs.Num()>=1) {
							Info+=(FString(TEXT("Outputs:\n"))); Nill=false;
							for (const FString &Output : FunInfo.Outputs) {
								Info += (FString(TEXT("\t"))+Output+FString(TEXT("\n")));
							} Info += FString(TEXT("\n"));
						}///
						//
						if (Info.Contains(TEXT("Return"))) {Info+=FString(TEXT("\n"));}
						if (!FunInfo.Tooltip.IsEmpty()) {Info+=(FunInfo.Tooltip+FString(TEXT("\n"))); Nill=false;}
						//
						if (!Nill) {KNode->SetTooltip(Info);} else {KNode->SetTooltip(Keyword);}
					}///
				}///
			} else {
				if (Source==ESKMGC_Source::Header) {KNode->SetTooltip(Script->Source.Script);}
				if (Source==ESKMGC_Source::Script) {KNode->SetTooltip(Script->Source.Header);}
			}///
		}///
	} else if (!HasKeyboardFocus()&&(KNode->GetTooltipText().ToString()!=Script->Source.Script)) {
		if (Source==ESKMGC_Source::Header) {KNode->SetTooltip(Script->Source.Script);}
		if (Source==ESKMGC_Source::Script) {KNode->SetTooltip(Script->Source.Header);}
	}///
	//
	return SGraphNode::OnMouseMove(MyGeometry,MouseEvent);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#undef LOCTEXT_NAMESPACE

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////