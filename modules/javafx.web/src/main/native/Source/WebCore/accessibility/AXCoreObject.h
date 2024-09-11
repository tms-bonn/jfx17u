/*
 * Copyright (C) 2019 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. AND ITS CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL APPLE INC. OR ITS CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#include "CharacterRange.h"
#include "ColorConversion.h"
#include "HTMLTextFormControlElement.h"
#include "LayoutRect.h"
#include "LocalFrameLoaderClient.h"
#include "LocalizedStrings.h"
#include "SimpleRange.h"
#include "TextIteratorBehavior.h"
#include "VisibleSelection.h"
#include "Widget.h"
#include <variant>
#include <wtf/HashSet.h>
#include <wtf/ObjectIdentifier.h>
#include <wtf/ProcessID.h>
#include <wtf/RefCounted.h>
#include <wtf/ThreadSafeWeakPtr.h>

#if PLATFORM(WIN)
#include "AccessibilityObjectWrapperWin.h"
#include "COMPtr.h"
#endif

#if USE(ATSPI)
#include "AccessibilityObjectAtspi.h"
#endif

#if PLATFORM(COCOA)
OBJC_CLASS WebAccessibilityObjectWrapper;
typedef WebAccessibilityObjectWrapper AccessibilityObjectWrapper;
typedef struct _NSRange NSRange;
typedef const struct __AXTextMarker* AXTextMarkerRef;
typedef const struct __AXTextMarkerRange* AXTextMarkerRangeRef;
OBJC_CLASS NSAttributedString;
#elif USE(ATSPI)

namespace WebCore {
class AccessibilityObjectAtspi;
}
typedef WebCore::AccessibilityObjectAtspi AccessibilityObjectWrapper;

#elif PLATFORM(PLAYSTATION)
class AccessibilityObjectWrapper : public RefCounted<AccessibilityObjectWrapper> { };
#else
class AccessibilityObjectWrapper;
#endif

namespace WTF {
class TextStream;
}

namespace WebCore {

class AXCoreObject;
class AXObjectCache;
class AXTextMarkerRange;
class AccessibilityScrollView;
class Document;
class Element;
class LocalFrame;
class LocalFrameView;
class Node;
class Page;
class Path;
class QualifiedName;
class RenderObject;
class ScrollView;

struct AccessibilityText;
struct CharacterRange;
struct ScrollRectToVisibleOptions;

enum class AXIDType { };
using AXID = ObjectIdentifier<AXIDType>;

enum class AXAncestorFlag : uint8_t {
    // When the flags aren't initialized, it means the object hasn't been inserted into the tree,
    // and thus we haven't set any of these ancestry flags.
    FlagsInitialized = 1 << 0,
    HasDocumentRoleAncestor = 1 << 1,
    HasWebApplicationAncestor = 1 << 2,
    IsInDescriptionListDetail = 1 << 3,
    IsInDescriptionListTerm = 1 << 4,
    IsInCell = 1 << 5,
    IsInRow = 1 << 6,

    // Bit 7 is free.
};

enum class AccessibilityRole {
    Application = 1,
    ApplicationAlert,
    ApplicationAlertDialog,
    ApplicationDialog,
    ApplicationGroup,
    ApplicationLog,
    ApplicationMarquee,
    ApplicationStatus,
    ApplicationTextGroup,
    ApplicationTimer,
    Audio,
    Blockquote,
    Button,
    Canvas,
    Caption,
    Cell,
    Checkbox,
    Code,
    ColorWell,
    Column,
    ColumnHeader,
    ComboBox,
    DateTime,
    Definition,
    Deletion,
    DescriptionList,
    DescriptionListDetail,
    DescriptionListTerm,
    Details,
    Directory,
    Document,
    DocumentArticle,
    DocumentMath,
    DocumentNote,
    Feed,
    Figure,
    Footer,
    Footnote,
    Form,
    Generic,
    GraphicsDocument,
    GraphicsObject,
    GraphicsSymbol,
    Grid,
    GridCell,
    Group,
    Heading,
    HorizontalRule,
    Ignored,
    Inline,
    Image,
    ImageMap,
    ImageMapLink,
    Incrementor,
    Insertion,
    Label,
    LandmarkBanner,
    LandmarkComplementary,
    LandmarkContentInfo,
    LandmarkDocRegion,
    LandmarkMain,
    LandmarkNavigation,
    LandmarkRegion,
    LandmarkSearch,
    Legend,
    LineBreak,
    Link,
    List,
    ListBox,
    ListBoxOption,
    ListItem,
    ListMarker,
    Mark,
    MathElement,
    Menu,
    MenuBar,
    MenuButton,
    MenuItem,
    MenuItemCheckbox,
    MenuItemRadio,
    MenuListPopup,
    MenuListOption,
    Meter,
    Model,
    Paragraph,
    PopUpButton,
    Pre,
    Presentational,
    ProgressIndicator,
    RadioButton,
    RadioGroup,
    RowHeader,
    Row,
    RowGroup,
    RubyBase,
    RubyBlock,
    RubyInline,
    RubyRun,
    RubyText,
    ScrollArea,
    ScrollBar,
    SearchField,
    Slider,
    SliderThumb,
    SpinButton,
    SpinButtonPart,
    Splitter,
    StaticText,
    Subscript,
    Suggestion,
    Summary,
    Superscript,
    Switch,
    SVGRoot,
    SVGText,
    SVGTSpan,
    SVGTextPath,
    TabGroup,
    TabList,
    TabPanel,
    Tab,
    Table,
    TableHeaderContainer,
    Term,
    TextArea,
    TextField,
    TextGroup,
    Time,
    Tree,
    TreeGrid,
    TreeItem,
    ToggleButton,
    Toolbar,
    Unknown,
    UserInterfaceTooltip,
    Video,
    WebApplication,
    WebArea,
    WebCoreLink,
};

using AccessibilityRoleSet = HashSet<AccessibilityRole, IntHash<AccessibilityRole>, WTF::StrongEnumHashTraits<AccessibilityRole>>;

ALWAYS_INLINE String accessibilityRoleToString(AccessibilityRole role)
{
    switch (role) {
    case AccessibilityRole::Application:
        return "Application"_s;
    case AccessibilityRole::ApplicationAlert:
        return "ApplicationAlert"_s;
    case AccessibilityRole::ApplicationAlertDialog:
        return "ApplicationAlertDialog"_s;
    case AccessibilityRole::ApplicationDialog:
        return "ApplicationDialog"_s;
    case AccessibilityRole::ApplicationGroup:
        return "ApplicationGroup"_s;
    case AccessibilityRole::ApplicationLog:
        return "ApplicationLog"_s;
    case AccessibilityRole::ApplicationMarquee:
        return "ApplicationMarquee"_s;
    case AccessibilityRole::ApplicationStatus:
        return "ApplicationStatus"_s;
    case AccessibilityRole::ApplicationTextGroup:
        return "ApplicationTextGroup"_s;
    case AccessibilityRole::ApplicationTimer:
        return "ApplicationTimer"_s;
    case AccessibilityRole::Audio:
        return "Audio"_s;
    case AccessibilityRole::Blockquote:
        return "Blockquote"_s;
    case AccessibilityRole::Button:
        return "Button"_s;
    case AccessibilityRole::Canvas:
        return "Canvas"_s;
    case AccessibilityRole::Caption:
        return "Caption"_s;
    case AccessibilityRole::Cell:
        return "Cell"_s;
    case AccessibilityRole::Checkbox:
        return "Checkbox"_s;
    case AccessibilityRole::Code:
        return "Code"_s;
    case AccessibilityRole::ColorWell:
        return "ColorWell"_s;
    case AccessibilityRole::Column:
        return "Column"_s;
    case AccessibilityRole::ColumnHeader:
        return "ColumnHeader"_s;
    case AccessibilityRole::ComboBox:
        return "ComboBox"_s;
    case AccessibilityRole::DateTime:
        return "DateTime"_s;
    case AccessibilityRole::Definition:
        return "Definition"_s;
    case AccessibilityRole::Deletion:
        return "Deletion"_s;
    case AccessibilityRole::DescriptionList:
        return "DescriptionList"_s;
    case AccessibilityRole::DescriptionListTerm:
        return "DescriptionListTerm"_s;
    case AccessibilityRole::DescriptionListDetail:
        return "DescriptionListDetail"_s;
    case AccessibilityRole::Details:
        return "Details"_s;
    case AccessibilityRole::Directory:
        return "Directory"_s;
    case AccessibilityRole::Document:
        return "Document"_s;
    case AccessibilityRole::DocumentArticle:
        return "DocumentArticle"_s;
    case AccessibilityRole::DocumentMath:
        return "DocumentMath"_s;
    case AccessibilityRole::DocumentNote:
        return "DocumentNote"_s;
    case AccessibilityRole::Feed:
        return "Feed"_s;
    case AccessibilityRole::Figure:
        return "Figure"_s;
    case AccessibilityRole::Footer:
        return "Footer"_s;
    case AccessibilityRole::Footnote:
        return "Footnote"_s;
    case AccessibilityRole::Form:
        return "Form"_s;
    case AccessibilityRole::Generic:
        return "Generic"_s;
    case AccessibilityRole::GraphicsDocument:
        return "GraphicsDocument"_s;
    case AccessibilityRole::GraphicsObject:
        return "GraphicsObject"_s;
    case AccessibilityRole::GraphicsSymbol:
        return "GraphicsSymbol"_s;
    case AccessibilityRole::Grid:
        return "Grid"_s;
    case AccessibilityRole::GridCell:
        return "GridCell"_s;
    case AccessibilityRole::Group:
        return "Group"_s;
    case AccessibilityRole::Heading:
        return "Heading"_s;
    case AccessibilityRole::HorizontalRule:
        return "HorizontalRule"_s;
    case AccessibilityRole::Ignored:
        return "Ignored"_s;
    case AccessibilityRole::Inline:
        return "Inline"_s;
    case AccessibilityRole::Image:
        return "Image"_s;
    case AccessibilityRole::ImageMap:
        return "ImageMap"_s;
    case AccessibilityRole::ImageMapLink:
        return "ImageMapLink"_s;
    case AccessibilityRole::Incrementor:
        return "Incrementor"_s;
    case AccessibilityRole::Insertion:
        return "Insertion"_s;
    case AccessibilityRole::Label:
        return "Label"_s;
    case AccessibilityRole::LandmarkBanner:
        return "LandmarkBanner"_s;
    case AccessibilityRole::LandmarkComplementary:
        return "LandmarkComplementary"_s;
    case AccessibilityRole::LandmarkContentInfo:
        return "LandmarkContentInfo"_s;
    case AccessibilityRole::LandmarkDocRegion:
        return "LandmarkDocRegion"_s;
    case AccessibilityRole::LandmarkMain:
        return "LandmarkMain"_s;
    case AccessibilityRole::LandmarkNavigation:
        return "LandmarkNavigation"_s;
    case AccessibilityRole::LandmarkRegion:
        return "LandmarkRegion"_s;
    case AccessibilityRole::LandmarkSearch:
        return "LandmarkSearch"_s;
    case AccessibilityRole::Legend:
        return "Legend"_s;
    case AccessibilityRole::Link:
        return "Link"_s;
    case AccessibilityRole::LineBreak:
        return "LineBreak"_s;
    case AccessibilityRole::List:
        return "List"_s;
    case AccessibilityRole::ListBox:
        return "ListBox"_s;
    case AccessibilityRole::ListBoxOption:
        return "ListBoxOption"_s;
    case AccessibilityRole::ListItem:
        return "ListItem"_s;
    case AccessibilityRole::ListMarker:
        return "ListMarker"_s;
    case AccessibilityRole::Mark:
        return "Mark"_s;
    case AccessibilityRole::MathElement:
        return "MathElement"_s;
    case AccessibilityRole::Menu:
        return "Menu"_s;
    case AccessibilityRole::MenuBar:
        return "MenuBar"_s;
    case AccessibilityRole::MenuButton:
        return "MenuButton"_s;
    case AccessibilityRole::MenuItem:
        return "MenuItem"_s;
    case AccessibilityRole::MenuItemCheckbox:
        return "MenuItemCheckbox"_s;
    case AccessibilityRole::MenuItemRadio:
        return "MenuItemRadio"_s;
    case AccessibilityRole::MenuListPopup:
        return "MenuListPopup"_s;
    case AccessibilityRole::MenuListOption:
        return "MenuListOption"_s;
    case AccessibilityRole::Meter:
        return "Meter"_s;
    case AccessibilityRole::Model:
        return "Model"_s;
    case AccessibilityRole::Paragraph:
        return "Paragraph"_s;
    case AccessibilityRole::PopUpButton:
        return "PopUpButton"_s;
    case AccessibilityRole::Pre:
        return "Pre"_s;
    case AccessibilityRole::Presentational:
        return "Presentational"_s;
    case AccessibilityRole::ProgressIndicator:
        return "ProgressIndicator"_s;
    case AccessibilityRole::RadioButton:
        return "RadioButton"_s;
    case AccessibilityRole::RadioGroup:
        return "RadioGroup"_s;
    case AccessibilityRole::RowHeader:
        return "RowHeader"_s;
    case AccessibilityRole::Row:
        return "Row"_s;
    case AccessibilityRole::RowGroup:
        return "RowGroup"_s;
    case AccessibilityRole::RubyBase:
        return "RubyBase"_s;
    case AccessibilityRole::RubyBlock:
        return "RubyBlock"_s;
    case AccessibilityRole::RubyInline:
        return "RubyInline"_s;
    case AccessibilityRole::RubyRun:
        return "RubyRun"_s;
    case AccessibilityRole::RubyText:
        return "RubyText"_s;
    case AccessibilityRole::ScrollArea:
        return "ScrollArea"_s;
    case AccessibilityRole::ScrollBar:
        return "ScrollBar"_s;
    case AccessibilityRole::SearchField:
        return "SearchField"_s;
    case AccessibilityRole::Slider:
        return "Slider"_s;
    case AccessibilityRole::SliderThumb:
        return "SliderThumb"_s;
    case AccessibilityRole::SpinButton:
        return "SpinButton"_s;
    case AccessibilityRole::SpinButtonPart:
        return "SpinButtonPart"_s;
    case AccessibilityRole::Splitter:
        return "Splitter"_s;
    case AccessibilityRole::StaticText:
        return "StaticText"_s;
    case AccessibilityRole::Subscript:
        return "Subscript"_s;
    case AccessibilityRole::Suggestion:
        return "Suggestion"_s;
    case AccessibilityRole::Summary:
        return "Summary"_s;
    case AccessibilityRole::Superscript:
        return "Superscript"_s;
    case AccessibilityRole::Switch:
        return "Switch"_s;
    case AccessibilityRole::SVGRoot:
        return "SVGRoot"_s;
    case AccessibilityRole::SVGText:
        return "SVGText"_s;
    case AccessibilityRole::SVGTSpan:
        return "SVGTSpan"_s;
    case AccessibilityRole::SVGTextPath:
        return "SVGTextPath"_s;
    case AccessibilityRole::TabGroup:
        return "TabGroup"_s;
    case AccessibilityRole::TabList:
        return "TabList"_s;
    case AccessibilityRole::TabPanel:
        return "TabPanel"_s;
    case AccessibilityRole::Tab:
        return "Tab"_s;
    case AccessibilityRole::Table:
        return "Table"_s;
    case AccessibilityRole::TableHeaderContainer:
        return "TableHeaderContainer"_s;
    case AccessibilityRole::Term:
        return "Term"_s;
    case AccessibilityRole::TextArea:
        return "TextArea"_s;
    case AccessibilityRole::TextField:
        return "TextField"_s;
    case AccessibilityRole::TextGroup:
        return "TextGroup"_s;
    case AccessibilityRole::Time:
        return "Time"_s;
    case AccessibilityRole::Tree:
        return "Tree"_s;
    case AccessibilityRole::TreeGrid:
        return "TreeGrid"_s;
    case AccessibilityRole::TreeItem:
        return "TreeItem"_s;
    case AccessibilityRole::ToggleButton:
        return "ToggleButton"_s;
    case AccessibilityRole::Toolbar:
        return "Toolbar"_s;
    case AccessibilityRole::Unknown:
        return "Unknown"_s;
    case AccessibilityRole::UserInterfaceTooltip:
        return "UserInterfaceTooltip"_s;
    case AccessibilityRole::Video:
        return "Video"_s;
    case AccessibilityRole::WebApplication:
        return "WebApplication"_s;
    case AccessibilityRole::WebArea:
        return "WebArea"_s;
    case AccessibilityRole::WebCoreLink:
        return "WebCoreLink"_s;
    }
    UNREACHABLE();
    return ""_s;
}

enum class AccessibilityDetachmentType { CacheDestroyed, ElementDestroyed, ElementChanged };

enum class AccessibilityConversionSpace { Screen, Page };

// FIXME: This should be replaced by AXDirection (or vice versa).
enum class AccessibilitySearchDirection {
    Next = 1,
    Previous,
};

enum class AccessibilityTextSource {
    Alternative,
    Children,
    Summary,
    Help,
    Visible,
    TitleTag,
    Placeholder,
    LabelByElement,
    Title,
    Subtitle,
    Action,
};

enum class AccessibilitySearchKey {
    AnyType = 1,
    Article,
    BlockquoteSameLevel,
    Blockquote,
    BoldFont,
    Button,
    Checkbox,
    Control,
    DifferentType,
    FontChange,
    FontColorChange,
    Frame,
    Graphic,
#if ENABLE(AX_THREAD_TEXT_APIS)
    HasTextRuns,
#endif
    HeadingLevel1,
    HeadingLevel2,
    HeadingLevel3,
    HeadingLevel4,
    HeadingLevel5,
    HeadingLevel6,
    HeadingSameLevel,
    Heading,
    Highlighted,
    ItalicFont,
    KeyboardFocusable,
    Landmark,
    Link,
    List,
    LiveRegion,
    MisspelledWord,
    Outline,
    PlainText,
    RadioGroup,
    SameType,
    StaticText,
    StyleChange,
    TableSameLevel,
    Table,
    TextField,
    Underline,
    UnvisitedLink,
    VisitedLink,
};

using AXEditingStyleValueVariant = std::variant<String, bool, int>;

struct AccessibilitySearchCriteria {
    AXCoreObject* anchorObject { nullptr };
    AXCoreObject* startObject;
    AXID stopAtID;
    AccessibilitySearchDirection searchDirection;
    Vector<AccessibilitySearchKey> searchKeys;
    String searchText;
    unsigned resultsLimit;
    bool visibleOnly;
    bool immediateDescendantsOnly;

    AccessibilitySearchCriteria(AXCoreObject* startObject, AccessibilitySearchDirection searchDirection, String searchText, unsigned resultsLimit, bool visibleOnly, bool immediateDescendantsOnly)
        : startObject(startObject)
        , searchDirection(searchDirection)
        , searchText(searchText)
        , resultsLimit(resultsLimit)
        , visibleOnly(visibleOnly)
        , immediateDescendantsOnly(immediateDescendantsOnly)
    { }
};

enum class AccessibilityObjectInclusion {
    IncludeObject,
    IgnoreObject,
    DefaultBehavior,
};

enum class AccessibilityCurrentState { False, True, Page, Step, Location, Date, Time };

enum class AccessibilityButtonState {
    Off = 0,
    On,
    Mixed,
};

enum class AXDirection : bool { Next, Previous };

enum class AccessibilitySortDirection {
    None,
    Ascending,
    Descending,
    Other,
    Invalid,
};

enum class AccessibilitySearchTextStartFrom {
    Begin, // Search from the beginning of the element.
    Selection, // Search from the position of the current selection.
    End // Search from the end of the element.
};

enum class AccessibilitySearchTextDirection {
    Forward, // Occurrence after the starting range.
    Backward, // Occurrence before the starting range.
    Closest, // Closest occurrence to the starting range, whether after or before.
    All // All occurrences
};

struct AccessibilitySearchTextCriteria {
    Vector<String> searchStrings; // Text strings to search for.
    AccessibilitySearchTextStartFrom start;
    AccessibilitySearchTextDirection direction;

    AccessibilitySearchTextCriteria()
        : start(AccessibilitySearchTextStartFrom::Selection)
        , direction(AccessibilitySearchTextDirection::Forward)
    { }
};

struct AccessibilityText {
    String text;
    AccessibilityTextSource textSource;

    AccessibilityText(const String& text, const AccessibilityTextSource& source)
        : text(text)
        , textSource(source)
    { }
};

enum class AccessibilityTextOperationType {
    Select,
    Replace,
    Capitalize,
    Lowercase,
    Uppercase
};

struct AccessibilityTextOperation {
    Vector<SimpleRange> textRanges; // text on which perform the operation.
    AccessibilityTextOperationType type;
    String replacementText; // For type = replace.

    AccessibilityTextOperation()
        : type(AccessibilityTextOperationType::Select)
    { }
};

enum class AccessibilityOrientation {
    Vertical,
    Horizontal,
    Undefined,
};

struct AccessibilityTextUnderElementMode {
    enum ChildrenInclusion {
        TextUnderElementModeSkipIgnoredChildren,
        TextUnderElementModeIncludeAllChildren,
        TextUnderElementModeIncludeNameFromContentsChildren, // This corresponds to ARIA concept: nameFrom
    };

    ChildrenInclusion childrenInclusion;
    bool includeFocusableContent;
    Node* ignoredChildNode;

    AccessibilityTextUnderElementMode(ChildrenInclusion c = TextUnderElementModeSkipIgnoredChildren, bool i = false, Node* ignored = nullptr)
        : childrenInclusion(c)
        , includeFocusableContent(i)
        , ignoredChildNode(ignored)
    { }
};

enum class AccessibilityVisiblePositionForBounds {
    First,
    Last,
};

enum class AccessibilityMathScriptObjectType { Subscript, Superscript };
enum class AccessibilityMathMultiscriptObjectType { PreSubscript, PreSuperscript, PostSubscript, PostSuperscript };

enum class CompositionState : uint8_t { Started, InProgress, Ended };

// Relationships between AX objects.
enum class AXRelationType : uint8_t {
    None,
    ActiveDescendant,
    ActiveDescendantOf,
    ControlledBy,
    ControllerFor,
    DescribedBy,
    DescriptionFor,
    Details,
    DetailsFor,
    ErrorMessage,
    ErrorMessageFor,
    FlowsFrom,
    FlowsTo,
    Headers,
    HeaderFor,
    LabeledBy,
    LabelFor,
    OwnedBy,
    OwnerFor,
};
using AXRelations = HashMap<AXRelationType, ListHashSet<AXID>, DefaultHash<uint8_t>, WTF::UnsignedWithZeroKeyHashTraits<uint8_t>>;

enum class SpinButtonType : bool {
    // The spin button is standalone. It has no separate controls, and should receive and perform actions itself.
    Standalone,
    // The spin button has separate increment and decrement controls.
    Composite
};

enum class ForceLayout : bool { No, Yes };

// Use this struct to store the isIgnored data that depends on the parents, so that in addChildren()
// we avoid going up the parent chain for each element while traversing the tree with useful information already.
struct AccessibilityIsIgnoredFromParentData {
    AXCoreObject* parent { nullptr };
    bool isAXHidden { false };
    bool isPresentationalChildOfAriaRole { false };
    bool isDescendantOfBarrenParent { false };

    AccessibilityIsIgnoredFromParentData(AXCoreObject* parent = nullptr)
        : parent(parent)
    { }

    bool isNull() const { return !parent; }
};

class AXCoreObject : public ThreadSafeRefCountedAndCanMakeThreadSafeWeakPtr<AXCoreObject> {
public:
    virtual ~AXCoreObject() = default;
    virtual String dbg() const = 0;

    void setObjectID(AXID axID) { m_id = axID; }
    AXID objectID() const { return m_id; }
    virtual AXID treeID() const = 0;
    virtual ProcessID processID() const = 0;

    // When the corresponding WebCore object that this accessible object
    // represents is deleted, it must be detached.
    void detach(AccessibilityDetachmentType);
    virtual bool isDetached() const = 0;

    typedef Vector<RefPtr<AXCoreObject>> AccessibilityChildrenVector;

    virtual bool isAccessibilityObject() const = 0;
    virtual bool isAccessibilityRenderObject() const = 0;
    virtual bool isAccessibilityTableInstance() const = 0;
    virtual bool isAccessibilityARIAGridInstance() const = 0;
    virtual bool isAccessibilityARIAGridRowInstance() const = 0;
    virtual bool isAccessibilityARIAGridCellInstance() const = 0;
    virtual bool isAXIsolatedObjectInstance() const = 0;

    bool isHeading() const { return roleValue() == AccessibilityRole::Heading; }
    virtual bool isLink() const = 0;
    bool isCode() const { return roleValue() == AccessibilityRole::Code; }
    bool isImage() const { return roleValue() == AccessibilityRole::Image; }
    bool isImageMap() const { return roleValue() == AccessibilityRole::ImageMap; }
    bool isVideo() const { return roleValue() == AccessibilityRole::Video; }
    virtual bool isSecureField() const = 0;
    virtual bool isNativeTextControl() const = 0;
    bool isWebArea() const { return roleValue() == AccessibilityRole::WebArea; }
    bool isCheckbox() const { return roleValue() == AccessibilityRole::Checkbox; }
    bool isRadioButton() const { return roleValue() == AccessibilityRole::RadioButton; }
    bool isListBox() const { return roleValue() == AccessibilityRole::ListBox; }
    virtual bool isListBoxOption() const = 0;
    virtual bool isAttachment() const = 0;
    bool isMenuRelated() const;
    bool isMenu() const { return roleValue() == AccessibilityRole::Menu; }
    bool isMenuBar() const { return roleValue() == AccessibilityRole::MenuBar; }
    bool isMenuButton() const { return roleValue() == AccessibilityRole::MenuButton; }
    bool isMenuItem() const;
    virtual bool isInputImage() const = 0;
    bool isProgressIndicator() const { return roleValue() == AccessibilityRole::ProgressIndicator || roleValue() == AccessibilityRole::Meter; }
    bool isSlider() const { return roleValue() == AccessibilityRole::Slider; }
    virtual bool isControl() const = 0;
    virtual bool isRadioInput() const = 0;
    // lists support (l, ul, ol, dl)
    virtual bool isList() const = 0;
    virtual bool isFileUploadButton() const = 0;

    // Table support.
    virtual bool isTable() const = 0;
    virtual bool isExposable() const = 0;
    unsigned tableLevel() const;
    virtual bool supportsSelectedRows() const = 0;
    virtual AccessibilityChildrenVector columns() = 0;
    virtual AccessibilityChildrenVector rows() = 0;
    virtual unsigned columnCount() = 0;
    virtual unsigned rowCount() = 0;
    // All the cells in the table.
    virtual AccessibilityChildrenVector cells() = 0;
    virtual AXCoreObject* cellForColumnAndRow(unsigned column, unsigned row) = 0;
    virtual AccessibilityChildrenVector columnHeaders() = 0;
    virtual AccessibilityChildrenVector rowHeaders() = 0;
    virtual AccessibilityChildrenVector visibleRows() = 0;
    AccessibilityChildrenVector selectedCells();
    // Returns an object that contains, as children, all the objects that act as headers.
    virtual AXCoreObject* headerContainer() = 0;
    virtual int axColumnCount() const = 0;
    virtual int axRowCount() const = 0;

    // Table cell support.
    virtual bool isTableCell() const = 0;
    virtual bool isExposedTableCell() const = 0;
    virtual bool isColumnHeader() const { return false; }
    virtual bool isRowHeader() const { return false; }
    bool isTableCellInSameRowGroup(AXCoreObject*);
    bool isTableCellInSameColGroup(AXCoreObject*);
    virtual AXID rowGroupAncestorID() const { return { }; }
    virtual String cellScope() const { return { }; }
    // Returns the start location and row span of the cell.
    virtual std::pair<unsigned, unsigned> rowIndexRange() const = 0;
    // Returns the start location and column span of the cell.
    virtual std::pair<unsigned, unsigned> columnIndexRange() const = 0;
    virtual int axColumnIndex() const = 0;
    virtual int axRowIndex() const = 0;

    // Table column support.
    virtual bool isTableColumn() const = 0;
    virtual unsigned columnIndex() const = 0;
    virtual AXCoreObject* columnHeader() = 0;

    // Table row support.
    virtual bool isTableRow() const = 0;
    virtual unsigned rowIndex() const = 0;
    virtual AXCoreObject* rowHeader() { return nullptr; }

    // ARIA tree/grid row support.
    virtual bool isARIATreeGridRow() const = 0;
    virtual AccessibilityChildrenVector disclosedRows() = 0; // Also implemented by ARIATreeItems.
    virtual AXCoreObject* disclosedByRow() const = 0;

    virtual bool isFieldset() const = 0;
    bool isGroup() const;

    // Native spin buttons.
    bool isSpinButton() const { return roleValue() == AccessibilityRole::SpinButton; }
    SpinButtonType spinButtonType();
    virtual AXCoreObject* incrementButton() = 0;
    virtual AXCoreObject* decrementButton() = 0;

    virtual bool isMockObject() const = 0;
    bool isSwitch() const { return roleValue() == AccessibilityRole::Switch; }
    bool isToggleButton() const { return roleValue() == AccessibilityRole::ToggleButton; }
    bool isTextControl() const;
    virtual bool isNonNativeTextControl() const = 0;
    bool isTabList() const { return roleValue() == AccessibilityRole::TabList; }
    bool isTabItem() const { return roleValue() == AccessibilityRole::Tab; }
    bool isRadioGroup() const { return roleValue() == AccessibilityRole::RadioGroup; }
    bool isComboBox() const { return roleValue() == AccessibilityRole::ComboBox; }
    bool isDateTime() const { return roleValue() == AccessibilityRole::DateTime; }
    bool isTree() const { return roleValue() == AccessibilityRole::Tree; }
    bool isTreeGrid() const { return roleValue() == AccessibilityRole::TreeGrid; }
    bool isTreeItem() const { return roleValue() == AccessibilityRole::TreeItem; }
    bool isScrollbar() const { return roleValue() == AccessibilityRole::ScrollBar; }
    bool isButton() const;
    virtual bool isMeter() const = 0;

    virtual HashMap<String, AXEditingStyleValueVariant> resolvedEditingStyles() const = 0;

    bool isListItem() const { return roleValue() == AccessibilityRole::ListItem; }
    bool isCheckboxOrRadio() const { return isCheckbox() || isRadioButton(); }
    bool isScrollView() const { return roleValue() == AccessibilityRole::ScrollArea; }
    bool isCanvas() const { return roleValue() == AccessibilityRole::Canvas; }
    bool isPopUpButton() const { return roleValue() == AccessibilityRole::PopUpButton; }
    bool isColorWell() const { return roleValue() == AccessibilityRole::ColorWell; }
    bool isSplitter() const { return roleValue() == AccessibilityRole::Splitter; }
    bool isToolbar() const { return roleValue() == AccessibilityRole::Toolbar; }
    bool isSummary() const { return roleValue() == AccessibilityRole::Summary; }
    bool isBlockquote() const { return roleValue() == AccessibilityRole::Blockquote; }
#if ENABLE(MODEL_ELEMENT)
    bool isModel() const { return roleValue() == AccessibilityRole::Model; }
#endif

    bool isLandmark() const;
    virtual bool isKeyboardFocusable() const = 0;

    virtual bool isChecked() const = 0;
    virtual bool isEnabled() const = 0;
    virtual bool isSelected() const = 0;
    virtual bool isFocused() const = 0;
    virtual bool isIndeterminate() const = 0;
    virtual bool isLoaded() const = 0;
    virtual bool isMultiSelectable() const = 0;
    // FIXME: should need just one since onscreen should be !offscreen.
    virtual bool isOnScreen() const = 0;
    virtual bool isOffScreen() const = 0;
    virtual bool isPressed() const = 0;
    virtual InsideLink insideLink() const = 0;
    bool isUnvisited() const { return insideLink() == InsideLink::InsideUnvisited; }
    bool isVisited() const { return insideLink() == InsideLink::InsideVisited; }
    virtual bool isRequired() const = 0;
    virtual bool supportsRequiredAttribute() const = 0;
    virtual bool isExpanded() const = 0;
    virtual bool isVisible() const = 0;
    virtual void setIsExpanded(bool) = 0;
    virtual bool supportsCheckedState() const = 0;

    // In a multi-select list, many items can be selected but only one is active at a time.
    virtual bool isSelectedOptionActive() const = 0;

    virtual bool hasBoldFont() const = 0;
    virtual bool hasItalicFont() const = 0;
    virtual bool hasMisspelling() const = 0;
    virtual std::optional<SimpleRange> misspellingRange(const SimpleRange& start, AccessibilitySearchDirection) const = 0;
    virtual std::optional<SimpleRange> visibleCharacterRange() const = 0;
    virtual bool hasPlainText() const = 0;
    virtual bool hasSameFont(const AXCoreObject&) const = 0;
    virtual bool hasSameFontColor(const AXCoreObject&) const = 0;
    virtual bool hasSameStyle(const AXCoreObject&) const = 0;
    bool isStaticText() const { return roleValue() == AccessibilityRole::StaticText; }
    virtual bool hasUnderline() const = 0;
    virtual bool hasHighlighting() const = 0;
    virtual AXTextMarkerRange textInputMarkedTextMarkerRange() const = 0;

    virtual bool supportsDatetimeAttribute() const = 0;
    virtual String datetimeAttributeValue() const = 0;

    virtual bool canSetFocusAttribute() const = 0;
    bool canSetTextRangeAttributes() const;
    virtual bool canSetValueAttribute() const = 0;
    bool canSetNumericValue() const { return roleValue() == AccessibilityRole::ScrollBar; }
    virtual bool canSetSelectedAttribute() const = 0;
    virtual bool canSetSelectedChildren() const = 0;
    bool canSetExpandedAttribute() const;

    virtual Element* element() const = 0;
    virtual Node* node() const = 0;
    virtual RenderObject* renderer() const = 0;

    virtual bool accessibilityIsIgnored() const = 0;

    virtual unsigned blockquoteLevel() const = 0;
    virtual unsigned headingLevel() const = 0;
    virtual AccessibilityButtonState checkboxOrRadioValue() const = 0;
    virtual String valueDescription() const = 0;
    virtual float valueForRange() const = 0;
    virtual float maxValueForRange() const = 0;
    virtual float minValueForRange() const = 0;
    AXCoreObject* selectedRadioButton();
    AXCoreObject* selectedTabItem();
    virtual int layoutCount() const = 0;
    virtual double loadingProgress() const = 0;
    virtual String brailleLabel() const = 0;
    virtual String brailleRoleDescription() const = 0;
    virtual String embeddedImageDescription() const = 0;
    virtual std::optional<AccessibilityChildrenVector> imageOverlayElements() = 0;
    virtual String extendedDescription() const = 0;

    virtual bool supportsARIAOwns() const = 0;

    // Retrieval of related objects.
    AXCoreObject* activeDescendant() const;
    AccessibilityChildrenVector activeDescendantOfObjects() const { return relatedObjects(AXRelationType::ActiveDescendantOf); }
    AccessibilityChildrenVector controlledObjects() const { return relatedObjects(AXRelationType::ControllerFor); }
    AccessibilityChildrenVector controllers() const { return relatedObjects(AXRelationType::ControlledBy); }
    AccessibilityChildrenVector describedByObjects() const { return relatedObjects(AXRelationType::DescribedBy); }
    AccessibilityChildrenVector descriptionForObjects() const { return relatedObjects(AXRelationType::DescriptionFor); }
    AccessibilityChildrenVector detailedByObjects() const { return relatedObjects(AXRelationType::Details); }
    AccessibilityChildrenVector detailsForObjects() const { return relatedObjects(AXRelationType::DetailsFor); }
    AccessibilityChildrenVector errorMessageObjects() const { return relatedObjects(AXRelationType::ErrorMessage); }
    AccessibilityChildrenVector errorMessageForObjects() const { return relatedObjects(AXRelationType::ErrorMessageFor); }
    AccessibilityChildrenVector flowToObjects() const { return relatedObjects(AXRelationType::FlowsTo); }
    AccessibilityChildrenVector flowFromObjects() const { return relatedObjects(AXRelationType::FlowsFrom); }
    AccessibilityChildrenVector labeledByObjects() const { return relatedObjects(AXRelationType::LabeledBy); }
    AccessibilityChildrenVector labelForObjects() const { return relatedObjects(AXRelationType::LabelFor); }
    AccessibilityChildrenVector ownedObjects() const { return relatedObjects(AXRelationType::OwnerFor); }
    AccessibilityChildrenVector owners() const { return relatedObjects(AXRelationType::OwnedBy); }
    virtual AccessibilityChildrenVector relatedObjects(AXRelationType) const = 0;

    virtual AXCoreObject* internalLinkElement() const = 0;
    void appendRadioButtonGroupMembers(AccessibilityChildrenVector& linkedUIElements) const;
    void appendRadioButtonDescendants(AXCoreObject&, AccessibilityChildrenVector&) const;
    virtual AccessibilityChildrenVector radioButtonGroup() const = 0;

    bool hasPopup() const;
    virtual String popupValue() const = 0;
    virtual bool supportsHasPopup() const = 0;
    virtual bool pressedIsPresent() const = 0;
    virtual String invalidStatus() const = 0;
    virtual bool supportsExpanded() const = 0;
    virtual bool supportsChecked() const = 0;
    virtual AccessibilitySortDirection sortDirection() const = 0;
    virtual bool supportsRangeValue() const = 0;
    virtual String identifierAttribute() const = 0;
    virtual String linkRelValue() const = 0;
    virtual Vector<String> classList() const = 0;
    virtual AccessibilityCurrentState currentState() const = 0;
    virtual bool supportsCurrent() const = 0;
    String currentValue() const;
    virtual bool supportsKeyShortcuts() const = 0;
    virtual String keyShortcuts() const = 0;

    virtual bool isModalNode() const = 0;

    virtual bool supportsSetSize() const = 0;
    virtual bool supportsPosInSet() const = 0;
    virtual int setSize() const = 0;
    virtual int posInSet() const = 0;

    // ARIA drag and drop
    virtual bool supportsDropping() const = 0;
    virtual bool supportsDragging() const = 0;
    virtual bool isGrabbed() = 0;
    virtual void setARIAGrabbed(bool) = 0;
    virtual Vector<String> determineDropEffects() const = 0;

    // Called on the root AX object to return the deepest available element.
    virtual AXCoreObject* accessibilityHitTest(const IntPoint&) const = 0;

    virtual AXCoreObject* focusedUIElement() const = 0;

#if PLATFORM(COCOA)
    virtual RemoteAXObjectRef remoteParentObject() const = 0;
#endif
    virtual AXCoreObject* parentObject() const = 0;
    virtual AXCoreObject* parentObjectUnignored() const = 0;

    virtual void findMatchingObjects(AccessibilitySearchCriteria*, AccessibilityChildrenVector&) = 0;
    virtual bool isDescendantOfRole(AccessibilityRole) const = 0;

    virtual bool hasDocumentRoleAncestor() const = 0;
    virtual bool hasWebApplicationAncestor() const = 0;
    virtual bool isInDescriptionListDetail() const = 0;
    virtual bool isInDescriptionListTerm() const = 0;
    virtual bool isInCell() const = 0;

    // Text selection
    virtual Vector<SimpleRange> findTextRanges(const AccessibilitySearchTextCriteria&) const = 0;
    virtual Vector<String> performTextOperation(const AccessibilityTextOperation&) = 0;

    AccessibilityChildrenVector linkedObjects() const;
    virtual AXCoreObject* titleUIElement() const;
    virtual AXCoreObject* scrollBar(AccessibilityOrientation) = 0;

    virtual bool inheritsPresentationalRole() const = 0;

    using AXValue = std::variant<bool, unsigned, float, String, AccessibilityButtonState, AXCoreObject*>;
    AXValue value();

    // Accessibility Text
    virtual void accessibilityText(Vector<AccessibilityText>&) const = 0;
    // A programmatic way to set a name on an AccessibleObject.
    virtual void setAccessibleName(const AtomString&) = 0;

    virtual String title() const = 0;
    virtual String description() const = 0;

    virtual std::optional<String> textContent() const = 0;
#if ENABLE(AX_THREAD_TEXT_APIS)
    virtual bool hasTextRuns() = 0;
#endif

    // Methods for determining accessibility text.
    virtual String stringValue() const = 0;
    virtual String textUnderElement(AccessibilityTextUnderElementMode = AccessibilityTextUnderElementMode()) const = 0;
    virtual String text() const = 0;
    virtual unsigned textLength() const = 0;
#if PLATFORM(COCOA)
    enum class SpellCheck : bool { No, Yes };
    virtual RetainPtr<NSAttributedString> attributedStringForTextMarkerRange(AXTextMarkerRange&&, SpellCheck) const = 0;
#endif
    virtual const String placeholderValue() const = 0;

    // Abbreviations
    virtual String expandedTextValue() const = 0;
    virtual bool supportsExpandedTextValue() const = 0;

    // Only if isColorWell()
    virtual SRGBA<uint8_t> colorValue() const = 0;

    virtual AccessibilityRole roleValue() const = 0;
    // Non-localized string associated with the object role.
    virtual String rolePlatformString() const = 0;
    // Localized string that describes the object's role.
    virtual String roleDescription() const = 0;
    // Localized string that describes ARIA landmark roles.
    String ariaLandmarkRoleDescription() const;
    // Non-localized string associated with the object's subrole.
    virtual String subrolePlatformString() const = 0;

    virtual AXObjectCache* axObjectCache() const = 0;

    virtual bool supportsPressAction() const = 0;
    virtual Element* actionElement() const = 0;

    // Rect relative to root document origin (i.e. absolute coordinates), disregarding viewport state.
    // This does not change when the viewport does (i.e via scrolling).
    virtual LayoutRect elementRect() const = 0;

    // Position relative to the viewport and normalized to screen coordinates.
    // Viewport-relative means that when the page scrolls, the portion of the page in the viewport changes, and thus
    // any viewport-relative rects do too (since they are either closer to or farther from the viewport origin after the scroll).
    virtual FloatPoint screenRelativePosition() const = 0;

    virtual FloatRect convertFrameToSpace(const FloatRect&, AccessibilityConversionSpace) const = 0;
#if PLATFORM(COCOA)
    virtual FloatRect convertRectToPlatformSpace(const FloatRect&, AccessibilityConversionSpace) const = 0;
#endif

    // Rect relative to the viewport.
    virtual FloatRect relativeFrame() const = 0;
#if PLATFORM(MAC)
    virtual FloatRect primaryScreenRect() const = 0;
#endif
    virtual FloatRect unobscuredContentRect() const = 0;
    virtual IntSize size() const = 0;
    virtual IntPoint clickPoint() = 0;
    virtual Path elementPath() const = 0;
    virtual bool supportsPath() const = 0;

    bool shouldReturnEmptySelectedText() const { return isSecureField(); }
    virtual CharacterRange selectedTextRange() const = 0;
    virtual int insertionPointLineNumber() const = 0;

    virtual URL url() const = 0;
    virtual VisibleSelection selection() const = 0;
    virtual String selectedText() const = 0;
    virtual String accessKey() const = 0;
    virtual String localizedActionVerb() const = 0;
    virtual String actionVerb() const = 0;

    // Widget support.
    virtual bool isWidget() const = 0;
    virtual Widget* widget() const = 0;
    virtual PlatformWidget platformWidget() const = 0;
    virtual Widget* widgetForAttachmentView() const = 0;
    virtual bool isPlugin() const = 0;

    // FIXME: Remove the following methods from the AXCoreObject interface and instead use methods such as axScrollView() if needed.
    virtual Page* page() const = 0;
    virtual Document* document() const = 0;
    virtual LocalFrameView* documentFrameView() const = 0;
    virtual ScrollView* scrollView() const = 0;
    // Should eliminate the need for exposing scrollView().
    AXCoreObject* axScrollView() const;

    virtual String language() const = 0;
    // 1-based, to match the aria-level spec.
    virtual unsigned hierarchicalLevel() const = 0;
    virtual bool isInlineText() const = 0;

    virtual void setFocused(bool) = 0;
    virtual void setSelectedText(const String&) = 0;
    virtual void setSelectedTextRange(CharacterRange&&) = 0;
    virtual bool setValue(const String&) = 0;
    virtual void setValueIgnoringResult(const String&) = 0;
    virtual bool replaceTextInRange(const String&, const CharacterRange&) = 0;
    virtual bool insertText(const String&) = 0;

    virtual bool setValue(float) = 0;
    virtual void setValueIgnoringResult(float) = 0;
    virtual void setSelected(bool) = 0;
    virtual void setSelectedRows(AccessibilityChildrenVector&&) = 0;

    virtual bool press() = 0;
    bool performDefaultAction() { return press(); }
    virtual bool performDismissAction() { return false; }
    virtual void performDismissActionIgnoringResult() = 0;

    virtual AccessibilityOrientation orientation() const = 0;
    virtual void increment() = 0;
    virtual void decrement() = 0;

    virtual const AccessibilityChildrenVector& children(bool updateChildrenIfNeeded = true) = 0;
    Vector<AXID> childrenIDs(bool updateChildrenIfNecessary = true);
    virtual void updateChildrenIfNecessary() = 0;
    virtual void detachFromParent() = 0;
    virtual bool isDetachedFromParent() = 0;

    bool canHaveSelectedChildren() const;
    virtual AccessibilityChildrenVector selectedChildren() = 0;
    virtual void setSelectedChildren(const AccessibilityChildrenVector&) = 0;
    virtual AccessibilityChildrenVector visibleChildren() = 0;
    AccessibilityChildrenVector tabChildren();
    bool isDescendantOfObject(const AXCoreObject*) const;
    bool isAncestorOfObject(const AXCoreObject*) const;

    virtual String nameAttribute() const = 0;
    virtual AtomString tagName() const = 0;

    virtual std::optional<SimpleRange> simpleRange() const = 0;
    virtual VisiblePositionRange visiblePositionRange() const = 0;
    virtual AXTextMarkerRange textMarkerRange() const = 0;

    virtual VisiblePositionRange visiblePositionRangeForLine(unsigned) const = 0;
    virtual VisiblePositionRange visiblePositionRangeForUnorderedPositions(const VisiblePosition&, const VisiblePosition&) const = 0;
    virtual VisiblePositionRange positionOfLeftWord(const VisiblePosition&) const = 0;
    virtual VisiblePositionRange positionOfRightWord(const VisiblePosition&) const = 0;
    virtual VisiblePositionRange leftLineVisiblePositionRange(const VisiblePosition&) const = 0;
    virtual VisiblePositionRange rightLineVisiblePositionRange(const VisiblePosition&) const = 0;
    virtual VisiblePositionRange sentenceForPosition(const VisiblePosition&) const = 0;
    virtual VisiblePositionRange paragraphForPosition(const VisiblePosition&) const = 0;
    virtual VisiblePositionRange styleRangeForPosition(const VisiblePosition&) const = 0;
    virtual VisiblePositionRange visiblePositionRangeForRange(const CharacterRange&) const = 0;
    virtual VisiblePositionRange lineRangeForPosition(const VisiblePosition&) const = 0;

    virtual std::optional<SimpleRange> rangeForCharacterRange(const CharacterRange&) const = 0;
#if PLATFORM(COCOA)
    virtual AXTextMarkerRange textMarkerRangeForNSRange(const NSRange&) const = 0;
#endif
#if PLATFORM(MAC)
    virtual AXTextMarkerRange selectedTextMarkerRange() = 0;
#endif

    virtual String stringForRange(const SimpleRange&) const = 0;
    virtual IntRect boundsForRange(const SimpleRange&) const = 0;
    virtual void setSelectedVisiblePositionRange(const VisiblePositionRange&) const = 0;

    virtual VisiblePosition visiblePositionForPoint(const IntPoint&) const = 0;
    virtual VisiblePosition nextLineEndPosition(const VisiblePosition&) const = 0;
    virtual VisiblePosition previousLineStartPosition(const VisiblePosition&) const = 0;
    virtual VisiblePosition nextSentenceEndPosition(const VisiblePosition&) const = 0;
    virtual VisiblePosition previousSentenceStartPosition(const VisiblePosition&) const = 0;
    virtual VisiblePosition nextParagraphEndPosition(const VisiblePosition&) const = 0;
    virtual VisiblePosition previousParagraphStartPosition(const VisiblePosition&) const = 0;
    virtual VisiblePosition visiblePositionForIndex(unsigned, bool /*lastIndexOK */) const = 0;

    virtual VisiblePosition visiblePositionForIndex(int) const = 0;
    virtual int indexForVisiblePosition(const VisiblePosition&) const = 0;

    virtual int lineForPosition(const VisiblePosition&) const = 0;

    virtual CharacterRange doAXRangeForLine(unsigned) const = 0;
    virtual CharacterRange characterRangeForPoint(const IntPoint&) const = 0;
    virtual CharacterRange doAXRangeForIndex(unsigned) const = 0;
    virtual CharacterRange doAXStyleRangeForIndex(unsigned) const = 0;

    virtual String doAXStringForRange(const CharacterRange&) const = 0;
    virtual IntRect doAXBoundsForRange(const CharacterRange&) const = 0;
    virtual IntRect doAXBoundsForRangeUsingCharacterOffset(const CharacterRange&) const = 0;

    virtual unsigned doAXLineForIndex(unsigned) = 0;

    virtual String computedRoleString() const = 0;

    virtual bool isValueAutofillAvailable() const = 0;
    virtual AutoFillButtonType valueAutofillButtonType() const = 0;

    // Used by an ARIA tree to get all its rows.
    virtual void ariaTreeRows(AccessibilityChildrenVector&) = 0;
    // Used by an ARIA tree item to get only its content, and not its child tree items and groups.
    AccessibilityChildrenVector ariaTreeItemContent();

    // ARIA live-region features.
    static bool liveRegionStatusIsEnabled(const AtomString&);
    bool supportsLiveRegion(bool excludeIfOff = true) const;
    virtual AXCoreObject* liveRegionAncestor(bool excludeIfOff = true) const = 0;
    bool isInsideLiveRegion(bool excludeIfOff = true) const;
    virtual const String liveRegionStatus() const = 0;
    virtual const String liveRegionRelevant() const = 0;
    virtual bool liveRegionAtomic() const = 0;
    virtual bool isBusy() const = 0;
    virtual String autoCompleteValue() const = 0;

    // Make this object visible by scrolling as many nested scrollable views as needed.
    virtual void scrollToMakeVisible() const = 0;
    // Same, but if the whole object can't be made visible, try for this subrect, in local coordinates.
    virtual void scrollToMakeVisibleWithSubFocus(IntRect&&) const = 0;
    // Scroll this object to a given point in global coordinates of the top-level window.
    virtual void scrollToGlobalPoint(IntPoint&&) const = 0;

    AccessibilityChildrenVector contents();

    // All math elements return true for isMathElement().
    virtual bool isMathElement() const = 0;
    virtual bool isMathFraction() const = 0;
    virtual bool isMathFenced() const = 0;
    virtual bool isMathSubscriptSuperscript() const = 0;
    virtual bool isMathRow() const = 0;
    virtual bool isMathUnderOver() const = 0;
    virtual bool isMathRoot() const = 0;
    virtual bool isMathSquareRoot() const = 0;
    virtual bool isMathTable() const = 0;
    virtual bool isMathTableRow() const = 0;
    virtual bool isMathTableCell() const = 0;
    virtual bool isMathMultiscript() const = 0;
    virtual bool isMathToken() const = 0;

    // Root components.
    virtual std::optional<AccessibilityChildrenVector> mathRadicand() = 0;
    virtual AXCoreObject* mathRootIndexObject() = 0;

    // Under over components.
    virtual AXCoreObject* mathUnderObject() = 0;
    virtual AXCoreObject* mathOverObject() = 0;

    // Fraction components.
    virtual AXCoreObject* mathNumeratorObject() = 0;
    virtual AXCoreObject* mathDenominatorObject() = 0;

    // Subscript/superscript components.
    virtual AXCoreObject* mathBaseObject() = 0;
    virtual AXCoreObject* mathSubscriptObject() = 0;
    virtual AXCoreObject* mathSuperscriptObject() = 0;

    // Fenced components.
    virtual String mathFencedOpenString() const = 0;
    virtual String mathFencedCloseString() const = 0;
    virtual int mathLineThickness() const = 0;

    // Multiscripts components.
    typedef std::pair<AXCoreObject*, AXCoreObject*> AccessibilityMathMultiscriptPair;
    typedef Vector<AccessibilityMathMultiscriptPair> AccessibilityMathMultiscriptPairs;
    virtual void mathPrescripts(AccessibilityMathMultiscriptPairs&) = 0;
    virtual void mathPostscripts(AccessibilityMathMultiscriptPairs&) = 0;
#if PLATFORM(JAVA)
    AccessibilityObjectWrapper* wrapper() const { return nullptr; }
    void setWrapper(AccessibilityObjectWrapper*) { }
    void detachWrapper(AccessibilityDetachmentType) { }
#else
    AccessibilityObjectWrapper* wrapper() const { return m_wrapper.get(); }
    void setWrapper(AccessibilityObjectWrapper* wrapper) { m_wrapper = wrapper; }
    void detachWrapper(AccessibilityDetachmentType);
#endif
#if PLATFORM(IOS_FAMILY)
    virtual int accessibilitySecureFieldLength() = 0;
    virtual bool hasTouchEventListener() const = 0;
#endif

    // allows for an AccessibilityObject to update its render tree or perform
    // other operations update type operations
    virtual void updateBackingStore() = 0;

#if PLATFORM(COCOA)
    virtual bool preventKeyboardDOMEventDispatch() const = 0;
    virtual void setPreventKeyboardDOMEventDispatch(bool) = 0;
    virtual String speechHintAttributeValue() const = 0;
    virtual bool fileUploadButtonReturnsValueInTitle() const = 0;
    String descriptionAttributeValue() const;
    bool shouldComputeDescriptionAttributeValue() const;
    String helpTextAttributeValue() const;
    // This should be the visible text that's actually on the screen if possible.
    // If there's alternative text, that can override the title.
    String titleAttributeValue() const;
    bool shouldComputeTitleAttributeValue() const;

    virtual bool hasApplePDFAnnotationAttribute() const = 0;
#endif

#if PLATFORM(MAC)
    virtual bool caretBrowsingEnabled() const = 0;
    virtual void setCaretBrowsingEnabled(bool) = 0;
#endif

    virtual AXCoreObject* focusableAncestor() = 0;
    virtual AXCoreObject* editableAncestor() = 0;
    virtual AXCoreObject* highestEditableAncestor() = 0;
    virtual AXCoreObject* exposedTableAncestor(bool includeSelf = false) const = 0;

    virtual AccessibilityChildrenVector documentLinks() = 0;

    virtual String innerHTML() const = 0;
    virtual String outerHTML() const = 0;

#if PLATFORM(COCOA) && ENABLE(MODEL_ELEMENT)
    virtual Vector<RetainPtr<id>> modelElementChildren() = 0;
#endif

protected:
    AXCoreObject() = default;
    explicit AXCoreObject(AXID axID)
        : m_id(axID)
    { }

private:
    // Detaches this object from the objects it references and it is referenced by.
    virtual void detachRemoteParts(AccessibilityDetachmentType) = 0;
#if !PLATFORM(JAVA)
    virtual void detachPlatformWrapper(AccessibilityDetachmentType) = 0;
#endif

    AXID m_id;
#if PLATFORM(COCOA)
    RetainPtr<WebAccessibilityObjectWrapper> m_wrapper;
#elif PLATFORM(WIN)
    COMPtr<AccessibilityObjectWrapper> m_wrapper;
#elif PLATFORM(PLAYSTATION)
    RefPtr<AccessibilityObjectWrapper> m_wrapper;
#elif USE(ATSPI)
    RefPtr<AccessibilityObjectAtspi> m_wrapper;
#endif
};

inline Vector<AXID> axIDs(const AXCoreObject::AccessibilityChildrenVector& objects)
{
    return objects.map([] (const auto& object) {
        return object ? object->objectID() : AXID();
    });
}

#if PLATFORM(COCOA)
inline bool AXCoreObject::shouldComputeDescriptionAttributeValue() const
{
    // Static text objects shouldn't return a description. Their content is communicated via AXValue.
    return roleValue() != AccessibilityRole::StaticText;
}

inline bool AXCoreObject::shouldComputeTitleAttributeValue() const
{
    // Static text objects shouldn't return a title. Their content is communicated via AXValue.
    return roleValue() != AccessibilityRole::StaticText;
}
#endif // PLATFORM(COCOA)

inline bool AXCoreObject::liveRegionStatusIsEnabled(const AtomString& liveRegionStatus)
{
    return equalLettersIgnoringASCIICase(liveRegionStatus, "polite"_s) || equalLettersIgnoringASCIICase(liveRegionStatus, "assertive"_s);
}

inline bool AXCoreObject::supportsLiveRegion(bool excludeIfOff) const
{
    auto liveRegionStatusValue = liveRegionStatus();
    return excludeIfOff ? liveRegionStatusIsEnabled(AtomString { liveRegionStatusValue }) : !liveRegionStatusValue.isEmpty();
}

inline SpinButtonType AXCoreObject::spinButtonType()
{
    ASSERT_WITH_MESSAGE(isSpinButton(), "spinButtonType() should only be called on spinbuttons.");
    return incrementButton() || decrementButton() ? SpinButtonType::Composite : SpinButtonType::Standalone;
}

inline bool AXCoreObject::isInsideLiveRegion(bool excludeIfOff) const
{
    return liveRegionAncestor(excludeIfOff);
}

inline bool AXCoreObject::canSetTextRangeAttributes() const
{
    return isTextControl();
}

inline bool AXCoreObject::canSetExpandedAttribute() const
{
    return supportsExpanded();
}

inline void AXCoreObject::detach(AccessibilityDetachmentType detachmentType)
{
    detachWrapper(detachmentType);
    if (detachmentType != AccessibilityDetachmentType::ElementChanged)
        detachRemoteParts(detachmentType);
}
#if !PLATFORM(JAVA)
inline void AXCoreObject::detachWrapper(AccessibilityDetachmentType detachmentType)
{
    detachPlatformWrapper(detachmentType);
    m_wrapper = nullptr;
}
#endif
inline Vector<AXID> AXCoreObject::childrenIDs(bool updateChildrenIfNecessary)
{
    return axIDs(children(updateChildrenIfNecessary));
}

namespace Accessibility {

template<typename T, typename F>
T* findAncestor(const T& object, bool includeSelf, const F& matches)
{
    T* parent;
    if (includeSelf)
        parent = const_cast<T*>(&object);
    else {
        auto* parentPtr = object.parentObject();
        if (!is<T>(parentPtr))
            return nullptr;
        parent = parentPtr;
    }

    for (; parent; parent = parent->parentObject()) {
        if (matches(*parent))
            return parent;
    }

    return nullptr;
}

template<typename T>
T* focusableAncestor(T& startObject)
{
    return findAncestor<T>(startObject, false, [] (const auto& ancestor) {
        return ancestor.canSetFocusAttribute();
    });
}

template<typename T>
T* editableAncestor(T& startObject)
{
    return findAncestor<T>(startObject, false, [] (const auto& ancestor) {
        return ancestor.isTextControl();
    });
}

template<typename T>
T* highestEditableAncestor(T& startObject)
{
    T* editableAncestor = startObject.editableAncestor();
    T* previousEditableAncestor = nullptr;
    while (editableAncestor) {
        if (editableAncestor == previousEditableAncestor) {
            if (T* parent = editableAncestor->parentObject()) {
                editableAncestor = parent->editableAncestor();
                continue;
            }
            break;
        }
        previousEditableAncestor = editableAncestor;
        editableAncestor = editableAncestor->editableAncestor();
    }
    return previousEditableAncestor;
}

template<typename T>
T* findRelatedObjectInAncestry(const T& object, AXRelationType relationType, const T& descendant)
{
    auto relatedObjects = object.relatedObjects(relationType);
    for (const auto& object : relatedObjects) {
        auto* ancestor = findAncestor(descendant, false, [&object] (const auto& ancestor) {
            return object.get() == &ancestor;
        });
        if (ancestor)
            return ancestor;
    }
    return nullptr;
}

template<typename T>
T* liveRegionAncestor(const T& object, bool excludeIfOff)
{
    return findAncestor<T>(object, true, [excludeIfOff] (const T& object) {
        return object.supportsLiveRegion(excludeIfOff);
    });
}

template<typename T>
T* exposedTableAncestor(const T& object, bool includeSelf = false)
{
    return findAncestor<T>(object, includeSelf, [] (const T& object) {
        return object.isTable() && object.isExposable();
    });
}

template<typename T, typename F>
T* findChild(T& object, F&& matches)
{
    for (auto child : object.children()) {
        if (matches(child)) {
            RELEASE_ASSERT(is<T>(child.get()));
            return downcast<T>(child.get());
        }
    }
    return nullptr;
}

void findMatchingObjects(const AccessibilitySearchCriteria&, AXCoreObject::AccessibilityChildrenVector&);

template<typename T, typename F>
void enumerateAncestors(const T& object, bool includeSelf, const F& lambda)
{
    if (includeSelf)
        lambda(object);

    if (auto* parent = object.parentObject())
        enumerateAncestors(*parent, true, lambda);
}

template<typename T, typename F>
void enumerateDescendants(T& object, bool includeSelf, const F& lambda)
{
    if (includeSelf)
        lambda(object);

    for (const auto& child : object.children()) {
        if (child)
            enumerateDescendants(*child, true, lambda);
    }
}

template<typename U> inline void performFunctionOnMainThreadAndWait(U&& lambda)
{
    callOnMainThreadAndWait([lambda = std::forward<U>(lambda)] () {
        lambda();
    });
}

template<typename U> inline void performFunctionOnMainThread(U&& lambda)
{
    ensureOnMainThread([lambda = std::forward<U>(lambda)] () mutable {
        lambda();
    });
}

template<typename T, typename U> inline T retrieveValueFromMainThread(U&& lambda)
{
    T value;
    callOnMainThreadAndWait([&value, &lambda] {
        value = lambda();
    });
    return value;
}

#if PLATFORM(COCOA)
template<typename T, typename U> inline T retrieveAutoreleasedValueFromMainThread(U&& lambda)
{
    RetainPtr<T> value;
    callOnMainThreadAndWait([&value, &lambda] {
        value = lambda();
    });
    return value.autorelease();
}
#endif

bool inRenderTreeOrStyleUpdate(const Document&);

} // namespace Accessibility

inline bool AXCoreObject::isDescendantOfObject(const AXCoreObject* axObject) const
{
    return axObject && Accessibility::findAncestor<AXCoreObject>(*this, false, [axObject] (const AXCoreObject& object) {
        return &object == axObject;
    }) != nullptr;
}

inline bool AXCoreObject::isAncestorOfObject(const AXCoreObject* axObject) const
{
    return axObject && (this == axObject || axObject->isDescendantOfObject(this));
}

inline AXCoreObject* AXCoreObject::axScrollView() const
{
    return Accessibility::findAncestor(*this, true, [] (const auto& ancestor) {
        return ancestor.isScrollView();
    });
}

// Logging helpers.
WTF::TextStream& operator<<(WTF::TextStream&, AccessibilityRole);
WTF::TextStream& operator<<(WTF::TextStream&, AccessibilitySearchDirection);
WTF::TextStream& operator<<(WTF::TextStream&, AccessibilitySearchKey);
WTF::TextStream& operator<<(WTF::TextStream&, const AccessibilitySearchCriteria&);
WTF::TextStream& operator<<(WTF::TextStream&, AccessibilityObjectInclusion);
WTF::TextStream& operator<<(WTF::TextStream&, const AXCoreObject&);
WTF::TextStream& operator<<(WTF::TextStream&, AccessibilityTextSource);
WTF::TextStream& operator<<(WTF::TextStream&, AXRelationType);

} // namespace WebCore
