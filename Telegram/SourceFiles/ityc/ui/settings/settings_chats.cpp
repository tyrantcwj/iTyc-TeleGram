// This is the source code of iTycGram for Desktop.
//
// We do not and cannot prevent the use of our code,
// but be respectful and credit the original author.
//
// Copyright @Radolyn, 2026
#include "ityc/ui/settings/settings_chats.h"

#include "lang_auto.h"
#include "ityc/ityc_settings.h"
#include "ityc/ui/boxes/edit_mark_box.h"
#include "ityc/ui/components/message_preview.h"
#include "ityc/ui/settings/ityc_builder.h"
#include "ityc/ui/settings/settings_ityc_utils.h"
#include "ityc/ui/settings/settings_main.h"
#include "settings/settings_builder.h"
#include "settings/settings_common.h"
#include "styles/style_ityc_icons.h"
#include "styles/style_menu_icons.h"
#include "styles/style_settings.h"
#include "ui/wrap/vertical_layout.h"
#include "window/window_session_controller.h"

#include <memory>

namespace Settings {

using namespace Builder;
using namespace AyuBuilder;

namespace {

struct PreviewState {
	MessagePreview *widget = nullptr;
};

void BuildStickersAndEmoji(SectionBuilder &builder, AyuSectionBuilder &ayu) {
	builder.addSubsectionTitle(tr::lng_settings_stickers_emoji());

	ayu.addSettingToggle({
		.id = u"ityc/showOnlyAddedEmojisAndStickers"_q,
		.title = tr::ityc_ShowOnlyAddedEmojisAndStickers(),
		.getter = &AyuSettings::showOnlyAddedEmojisAndStickers,
		.setter = &AyuSettings::setShowOnlyAddedEmojisAndStickers,
	});

	ayu.addSettingToggle({
		.id = u"ityc/unlimitedRecentStickers"_q,
		.altIds = { u"ityc/recentStickersCount"_q },
		.title = tr::ityc_SettingsUnlimitedRecentStickers(),
		.getter = &AyuSettings::unlimitedRecentStickers,
		.setter = &AyuSettings::setUnlimitedRecentStickers,
	});

	ayu.addCollapsibleToggle({
		.id = u"ityc/hideReactions"_q,
		.title = tr::ityc_HideReactions(),
		.checkboxes = {
			NestedEntry{
				tr::ityc_HideReactionsInChannels(tr::now),
				[] { return !AyuSettings::getInstance().showChannelReactions(); },
				[](bool v) { AyuSettings::getInstance().setShowChannelReactions(!v); }
			},
			NestedEntry{
				tr::ityc_HideReactionsInGroups(tr::now),
				[] { return !AyuSettings::getInstance().showGroupReactions(); },
				[](bool v) { AyuSettings::getInstance().setShowGroupReactions(!v); }
			},
			NestedEntry{
				tr::ityc_HideReactionsInPrivateChats(tr::now),
				[] { return !AyuSettings::getInstance().showPrivateChatReactions(); },
				[](bool v) { AyuSettings::getInstance().setShowPrivateChatReactions(!v); }
			}
		},
		.toggledWhenAll = false,
	});

	ayu.addSectionDivider();
}

void BuildGroupsAndChannels(SectionBuilder &builder, AyuSectionBuilder &ayu) {
	auto *settings = &AyuSettings::getInstance();

	builder.addSubsectionTitle(tr::lng_premium_double_limits_subtitle_channels());

	ayu.addChooseButton({
		.id = u"ityc/channelBottomButton"_q,
		.altIds = { u"ityc/bottomButton"_q },
		.title = tr::ityc_ChannelBottomButton(),
		.boxTitle = tr::ityc_ChannelBottomButton(),
		.initialSelection = static_cast<int>(settings->channelBottomButton()),
		.options = {
			tr::ityc_ChannelBottomButtonHide(tr::now),
			tr::ityc_ChannelBottomButtonMute(tr::now),
			tr::ityc_ChannelBottomButtonDiscuss(tr::now),
		},
		.setter = [](int index) {
			AyuSettings::getInstance().setChannelBottomButton(
				static_cast<ChannelBottomButton>(index));
		},
	});

	ayu.addSettingToggle({
		.id = u"ityc/quickAdminShortcuts"_q,
		.title = tr::ityc_QuickAdminShortcuts(),
		.getter = &AyuSettings::quickAdminShortcuts,
		.setter = &AyuSettings::setQuickAdminShortcuts,
	});
	ayu.addSettingToggle({
		.id = u"ityc/disableGreetingSticker"_q,
		.title = tr::ityc_DisableGreetingSticker(),
		.getter = &AyuSettings::disableGreetingSticker,
		.setter = &AyuSettings::setDisableGreetingSticker,
	});
	ayu.addSettingToggle({
		.id = u"ityc/showMessageShot"_q,
		.title = tr::ityc_SettingsShowMessageShot(),
		.getter = &AyuSettings::showMessageShot,
		.setter = &AyuSettings::setShowMessageShot,
	});

	builder.addSkip();
	builder.addDividerText(tr::ityc_SettingsShowMessageShotDescription());
	builder.addSkip();
}

void BuildMarks(
		SectionBuilder &builder,
		AyuSectionBuilder &ayu,
		std::shared_ptr<PreviewState> previewState) {
	auto *settings = &AyuSettings::getInstance();
	const auto controller = builder.controller();

	builder.addSubsectionTitle(tr::lng_settings_messages());

	builder.add([=](const WidgetContext &ctx) -> SectionBuilder::WidgetToAdd {
		auto preview = object_ptr<MessagePreview>(ctx.container, controller);
		previewState->widget = preview.data();
		return {
			.widget = std::move(preview),
			.margin = style::margins(
				0,
				st::defaultVerticalListSkip,
				0,
				st::settingsPrivacySkipTop),
		};
	});

	ayu.addSettingToggle({
		.id = u"ityc/replaceBottomInfoWithIcons"_q,
		.altIds = { u"ityc/replaceEditedWithIcon"_q },
		.title = tr::ityc_ReplaceMarksWithIcons(),
		.getter = &AyuSettings::replaceBottomInfoWithIcons,
		.setter = &AyuSettings::setReplaceBottomInfoWithIcons,
	});

	builder.scope([&] {
		builder.addButton({
			.id = u"ityc/deletedMark"_q,
			.title = tr::ityc_DeletedMarkText(),
			.st = &st::settingsButtonNoIcon,
			.label = AyuSettings::getInstance().deletedMarkValue(),
			.onClick = [=] {
				auto box = Box<EditMarkBox>(
					tr::ityc_DeletedMarkText(),
					settings->deletedMark(),
					QString("🧹"),
					[=](const QString &value) {
						AyuSettings::getInstance().setDeletedMark(value);
					});
				Ui::show(std::move(box));
			},
		});

		builder.addButton({
			.id = u"ityc/editedMark"_q,
			.title = tr::ityc_EditedMarkText(),
			.st = &st::settingsButtonNoIcon,
			.label = AyuSettings::getInstance().editedMarkValue(),
			.onClick = [=] {
				auto box = Box<EditMarkBox>(
					tr::ityc_EditedMarkText(),
					settings->editedMark(),
					tr::lng_edited(tr::now),
					[=](const QString &value) {
						AyuSettings::getInstance().setEditedMark(value);
					});
				Ui::show(std::move(box));
			},
		});
	}, AyuSettings::getInstance().replaceBottomInfoWithIconsValue()
		| rpl::map([](bool v) { return !v; }));

	ayu.addSettingToggle({
		.id = u"ityc/removeMessageTail"_q,
		.title = tr::ityc_RemoveMessageTail(),
		.getter = &AyuSettings::removeMessageTail,
		.setter = &AyuSettings::setRemoveMessageTail,
	});

	ayu.addSettingToggle({
		.id = u"ityc/hideFastShare"_q,
		.altIds = { u"ityc/hideShareButton"_q },
		.title = tr::ityc_HideShareButton(),
		.getter = &AyuSettings::hideFastShare,
		.setter = &AyuSettings::setHideFastShare,
	});
	ayu.addSettingToggle({
		.id = u"ityc/simpleQuotesAndReplies"_q,
		.altIds = { u"ityc/disableColorfulReplies"_q, u"ityc/replyElements"_q },
		.title = tr::ityc_SimpleQuotesAndReplies(),
		.getter = &AyuSettings::simpleQuotesAndReplies,
		.setter = &AyuSettings::setSimpleQuotesAndReplies,
	});

	const auto semiTransparent = ayu.addSettingToggle({
		.id = u"ityc/semiTransparentDeletedMessages"_q,
		.altIds = { u"ityc/translucentDeletedMessages"_q },
		.title = tr::ityc_SemiTransparentDeletedMessages(),
		.getter = &AyuSettings::semiTransparentDeletedMessages,
		.setter = &AyuSettings::setSemiTransparentDeletedMessages,
	});
	if (semiTransparent) {
		ayu.addBetaBadge(semiTransparent);
	}

	ayu.addSectionDivider();
}

void BuildWideMessagesMultiplier(
		SectionBuilder &builder,
		AyuSectionBuilder &ayu,
		std::shared_ptr<PreviewState> previewState) {
	auto *settings = &AyuSettings::getInstance();

	constexpr auto kMinSize = 1.00;
	constexpr auto kStep = 0.05;

	const auto valueToIndex = [=](double value) {
		return static_cast<int>(std::round((value - kMinSize) / kStep));
	};

	const auto controller = builder.controller();
	ayu.addSlider({
		.id = u"ityc/messageBubbleRadius"_q,
		.title = tr::ityc_MessageBubbleRadius(),
		.steps = 17,
		.current = settings->messageBubbleRadius(),
		.indexToValue = [](int index) { return index; },
		.onChanged = [=](int index) {
			if (previewState->widget) {
				previewState->widget->setBubbleRadius(index);
			}
		},
		.onFinalChanged = [=](int index) {
			if (previewState->widget) {
				previewState->widget->setBubbleRadius(index);
			}
			AyuSettings::getInstance().setMessageBubbleRadius(index);
			ShowRestartPrompt(controller);
		},
		.formatLabel = [](int index) {
			return QString::number(index);
		},
	});

	ayu.addSectionDivider();

	ayu.addSlider({
		.id = u"ityc/wideMultiplier"_q,
		.title = tr::ityc_SettingsWideMultiplier(),
		.steps = 61, // (4.00 - 1.00) / 0.05 + 1
		.current = valueToIndex(settings->wideMultiplier()),
		.indexToValue = [](int index) { return index; },
		.onChanged = nullptr,
		.onFinalChanged = [=](int index) {
			AyuSettings::getInstance().setWideMultiplier(
				kMinSize + index * kStep);
			ShowRestartPrompt(controller);
		},
		.formatLabel = [=](int index) {
			return QString::number(kMinSize + index * kStep, 'f', 2);
		},
	});

	builder.addSkip();
	builder.addDividerText(tr::ityc_SettingsWideMultiplierDescription());
	builder.addSkip();
}

void BuildContextMenuElements(SectionBuilder &builder, AyuSectionBuilder &ayu) {
	auto *settings = &AyuSettings::getInstance();

	builder.addSubsectionTitle(tr::ityc_ContextMenuElementsHeader());

	const auto options = std::vector{
		tr::ityc_SettingsContextMenuItemHidden(tr::now),
		tr::ityc_SettingsContextMenuItemShown(tr::now),
		tr::ityc_SettingsContextMenuItemExtended(tr::now),
	};

	ayu.addChooseButton({
		.id = u"ityc/showReactionsPanelInContextMenu"_q,
		.title = tr::ityc_SettingsContextMenuReactionsPanel(),
		.boxTitle = tr::ityc_SettingsContextMenuTitle(),
		.initialSelection = static_cast<int>(settings->showReactionsPanelInContextMenu()),
		.options = options,
		.setter = [](int i) { AyuSettings::getInstance().setShowReactionsPanelInContextMenu(static_cast<ContextMenuVisibility>(i)); },
		.icon = { &st::menuIconReactions },
	});
	ayu.addChooseButton({
		.id = u"ityc/showViewsPanelInContextMenu"_q,
		.title = tr::ityc_SettingsContextMenuViewsPanel(),
		.boxTitle = tr::ityc_SettingsContextMenuTitle(),
		.initialSelection = static_cast<int>(settings->showViewsPanelInContextMenu()),
		.options = options,
		.setter = [](int i) { AyuSettings::getInstance().setShowViewsPanelInContextMenu(static_cast<ContextMenuVisibility>(i)); },
		.icon = { &st::menuIconShowInChat },
	});
	ayu.addChooseButton({
		.id = u"ityc/showHideMessageInContextMenu"_q,
		.title = tr::ityc_ContextHideMessage(),
		.boxTitle = tr::ityc_SettingsContextMenuTitle(),
		.initialSelection = static_cast<int>(settings->showHideMessageInContextMenu()),
		.options = options,
		.setter = [](int i) { AyuSettings::getInstance().setShowHideMessageInContextMenu(static_cast<ContextMenuVisibility>(i)); },
		.icon = { &st::menuIconClear },
	});
	ayu.addChooseButton({
		.id = u"ityc/showUserMessagesInContextMenu"_q,
		.title = tr::ityc_UserMessagesMenuText(),
		.boxTitle = tr::ityc_SettingsContextMenuTitle(),
		.initialSelection = static_cast<int>(settings->showUserMessagesInContextMenu()),
		.options = options,
		.setter = [](int i) { AyuSettings::getInstance().setShowUserMessagesInContextMenu(static_cast<ContextMenuVisibility>(i)); },
		.icon = { &st::menuIconTTL },
	});
	ayu.addChooseButton({
		.id = u"ityc/showMessageDetailsInContextMenu"_q,
		.title = tr::ityc_MessageDetailsPC(),
		.boxTitle = tr::ityc_SettingsContextMenuTitle(),
		.initialSelection = static_cast<int>(settings->showMessageDetailsInContextMenu()),
		.options = options,
		.setter = [](int i) { AyuSettings::getInstance().setShowMessageDetailsInContextMenu(static_cast<ContextMenuVisibility>(i)); },
		.icon = { &st::menuIconInfo },
	});
	ayu.addChooseButton({
		.id = u"ityc/showRepeatMessageInContextMenu"_q,
		.title = tr::ityc_RepeatMessage(),
		.boxTitle = tr::ityc_SettingsContextMenuTitle(),
		.initialSelection = static_cast<int>(settings->showRepeatMessageInContextMenu()),
		.options = options,
		.setter = [](int i) { AyuSettings::getInstance().setShowRepeatMessageInContextMenu(static_cast<ContextMenuVisibility>(i)); },
		.icon = { &st::ayuRepeatMenuIcon },
	});
	if (settings->filtersEnabled()) {
		ayu.addChooseButton({
			.id = u"ityc/showAddFilterInContextMenu"_q,
			.title = tr::ityc_RegexFilterQuickAdd(),
			.boxTitle = tr::ityc_SettingsContextMenuTitle(),
			.initialSelection = static_cast<int>(settings->showAddFilterInContextMenu()),
			.options = options,
			.setter = [](int i) { AyuSettings::getInstance().setShowAddFilterInContextMenu(static_cast<ContextMenuVisibility>(i)); },
			.icon = { &st::menuIconAddToFolder },
		});
	}

	builder.addSkip();
	builder.addDividerText(tr::ityc_SettingsContextMenuDescription());
	builder.addSkip();
}

void BuildMessageFieldElements(SectionBuilder &builder, AyuSectionBuilder &ayu) {
	builder.addSubsectionTitle(tr::ityc_MessageFieldElementsHeader());

	ayu.addSettingToggle({
		.id = u"ityc/showAttachButtonInMessageField"_q,
		.title = tr::ityc_MessageFieldElementAttach(),
		.getter = &AyuSettings::showAttachButtonInMessageField,
		.setter = &AyuSettings::setShowAttachButtonInMessageField,
		.icon = { &st::messageFieldAttachIcon },
	});
	ayu.addSettingToggle({
		.id = u"ityc/showCommandsButtonInMessageField"_q,
		.title = tr::ityc_MessageFieldElementCommands(),
		.getter = &AyuSettings::showCommandsButtonInMessageField,
		.setter = &AyuSettings::setShowCommandsButtonInMessageField,
		.icon = { &st::messageFieldCommandsIcon },
	});
	ayu.addSettingToggle({
		.id = u"ityc/showAutoDeleteButtonInMessageField"_q,
		.title = tr::ityc_MessageFieldElementTTL(),
		.getter = &AyuSettings::showAutoDeleteButtonInMessageField,
		.setter = &AyuSettings::setShowAutoDeleteButtonInMessageField,
		.icon = { &st::messageFieldTTLIcon },
	});
	ayu.addSettingToggle({
		.id = u"ityc/showEmojiButtonInMessageField"_q,
		.title = tr::ityc_MessageFieldElementEmoji(),
		.getter = &AyuSettings::showEmojiButtonInMessageField,
		.setter = &AyuSettings::setShowEmojiButtonInMessageField,
		.icon = { &st::messageFieldEmojiIcon },
	});
	ayu.addSettingToggle({
		.id = u"ityc/showMicrophoneButtonInMessageField"_q,
		.title = tr::ityc_MessageFieldElementVoice(),
		.getter = &AyuSettings::showMicrophoneButtonInMessageField,
		.setter = &AyuSettings::setShowMicrophoneButtonInMessageField,
		.icon = { &st::messageFieldVoiceIcon },
	});
	ayu.addSettingToggle({
		.id = u"ityc/showGiftButtonInMessageField"_q,
		.title = tr::lng_profile_action_short_gift(),
		.getter = &AyuSettings::showGiftButtonInMessageField,
		.setter = &AyuSettings::setShowGiftButtonInMessageField,
		.icon = { &st::settingsButtonIconGift },
	});
	ayu.addSettingToggle({
		.id = u"ityc/showAiEditorButtonInMessageField"_q,
		.title = tr::lng_ai_compose_title(),
		.getter = &AyuSettings::showAiEditorButtonInMessageField,
		.setter = &AyuSettings::setShowAiEditorButtonInMessageField,
		.icon = { &st::messageFieldCocoonAiIcon },
	});

	ayu.addSectionDivider();
}

void BuildMessageFieldPopups(SectionBuilder &builder, AyuSectionBuilder &ayu) {
	builder.addSubsectionTitle(tr::ityc_MessageFieldPopupsHeader());

	ayu.addSettingToggle({
		.id = u"ityc/showAttachPopup"_q,
		.title = tr::ityc_MessageFieldElementAttach(),
		.getter = &AyuSettings::showAttachPopup,
		.setter = &AyuSettings::setShowAttachPopup,
		.icon = { &st::messageFieldAttachIcon },
	});
	ayu.addSettingToggle({
		.id = u"ityc/showEmojiPopup"_q,
		.title = tr::ityc_MessageFieldElementEmoji(),
		.getter = &AyuSettings::showEmojiPopup,
		.setter = &AyuSettings::setShowEmojiPopup,
		.icon = { &st::messageFieldEmojiIcon },
	});
}

const auto kMeta = BuildHelper({
	.id = AyuChats::Id(),
	.parentId = AyuMain::Id(),
	.title = &tr::ityc_CategoryChats,
	.icon = &st::menuIconChatBubble,
}, [](SectionBuilder &builder) {
	auto ayu = AyuSectionBuilder(builder);
	const auto previewState = std::make_shared<PreviewState>();

	builder.addSkip();
	BuildStickersAndEmoji(builder, ayu);
	BuildGroupsAndChannels(builder, ayu);
	BuildMarks(builder, ayu, previewState);
	BuildWideMessagesMultiplier(builder, ayu, previewState);
	BuildContextMenuElements(builder, ayu);
	BuildMessageFieldElements(builder, ayu);
	BuildMessageFieldPopups(builder, ayu);
	builder.addSkip();
});

} // namespace

rpl::producer<QString> AyuChats::title() {
	return tr::ityc_CategoryChats();
}

AyuChats::AyuChats(
	QWidget *parent,
	not_null<Window::SessionController*> controller)
: Section(parent, controller) {
	setupContent();
}

void AyuChats::setupContent() {
	const auto content = Ui::CreateChild<Ui::VerticalLayout>(this);
	build(content, kMeta.build);
	Ui::ResizeFitChild(this, content);
}

Type AyuChatsId() {
	return AyuChats::Id();
}

} // namespace Settings
