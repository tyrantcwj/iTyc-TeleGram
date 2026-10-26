// This is the source code of iTycGram for Desktop.
//
// We do not and cannot prevent the use of our code,
// but be respectful and credit the original author.
//
// Copyright @Radolyn, 2026
#include "ityc/ui/settings/settings_general.h"

#include "lang_auto.h"
#include "ityc/ityc_settings.h"
#include "ityc/ui/settings/ityc_builder.h"
#include "ityc/ui/settings/settings_ityc_utils.h"
#include "ityc/ui/settings/settings_main.h"
#include "base/platform/base_platform_info.h"
#include "core/application.h"
#include "lang/lang_text_entity.h"
#include "platform/platform_translate_provider.h"
#include "settings/settings_builder.h"
#include "settings/settings_common.h"
#include "styles/style_menu_icons.h"
#include "styles/style_settings.h"
#include "ui/boxes/single_choice_box.h"
#include "ui/toast/toast.h"
#include "ui/widgets/buttons.h"
#include "ui/wrap/vertical_layout.h"
#include "window/window_controller.h"
#include "window/window_session_controller.h"

namespace Settings {

using namespace Builder;
using namespace AyuBuilder;

namespace {

void BuildTranslator(SectionBuilder &builder, AyuSectionBuilder &ayu) {
	builder.addSubsectionTitle(tr::lng_translate_settings_subtitle());

	auto *settings = &AyuSettings::getInstance();

	const auto options = std::vector{
		std::pair(TranslationProvider::Telegram, QString("Telegram")),
		std::pair(TranslationProvider::Google, QString("Google")),
		std::pair(TranslationProvider::Yandex, QString("Yandex")),
	};
	const auto nativeAvailable = Platform::IsTranslateProviderAvailable();
	auto availableOptions = options;
	if (nativeAvailable) {
		availableOptions.push_back(std::pair(
			TranslationProvider::Native,
			[] {
				if constexpr (Platform::IsMac()) {
					return QString("macOS");
				} else if constexpr (Platform::IsWindows()) {
					return QString("Windows");
				} else {
					return QString("Linux");
				}
			}()));
	}
	auto optionLabels = std::vector<QString>();
	optionLabels.reserve(availableOptions.size());
	for (const auto &option : availableOptions) {
		optionLabels.push_back(option.second);
	}

	const auto getIndex = [=](TranslationProvider val) {
		const auto i = ranges::find(
			availableOptions,
			val,
			&std::pair<TranslationProvider, QString>::first);
		return (i != end(availableOptions))
			? int(i - begin(availableOptions))
			: 0;
	};

	auto currentVal = AyuSettings::getInstance().translationProviderValue()
		| rpl::map(getIndex)
		| rpl::map([=](int val) { return availableOptions[val].second; });

	const auto button = builder.addButton({
		.id = u"ityc/translationProvider"_q,
		.title = tr::ityc_TranslationProvider(),
		.st = &st::settingsButtonNoIcon,
		.label = std::move(currentVal),
		.onClick = [=] {
			if (const auto controller = Core::App().activeWindow()->sessionController()) {
				controller->show(Box(
						[=](not_null<Ui::GenericBox*> box) {
							const auto save = [=](int index) {
								const auto option = availableOptions[index].first;
								AyuSettings::getInstance().setTranslationProvider(option);

								if constexpr (Platform::IsMac()) {
									if (option == TranslationProvider::Native) {
										controller->showToast(Ui::Toast::Config{
											.text = tr::lng_translate_settings_use_platform_mac_about(tr::now, tr::rich),
											.duration = 6 * crl::time(1000)
										});
									}
								}
							};
							SingleChoiceBox(box, {
								.title = tr::ityc_TranslationProvider(),
								.options = optionLabels,
								.initialSelection = getIndex(settings->translationProvider()),
								.callback = save,
							});
						}));
			}
		},
	});
	if (button) {
		ayu.addBetaBadge(button);
	}
}

void BuildShowPeerId(SectionBuilder &builder) {
	auto *settings = &AyuSettings::getInstance();

	const auto options = std::vector{
		QString(tr::ityc_SettingsShowID_Hide(tr::now)),
		QString("Telegram API"),
		QString("Bot API")
	};

	auto currentVal = AyuSettings::getInstance().showPeerIdValue()
		| rpl::map([=](PeerIdDisplay val) {
			return options[static_cast<int>(val)];
		});

	const auto controller = builder.controller();
	builder.addButton({
		.id = u"ityc/showPeerId"_q,
		.altIds = { u"ityc/showIdAndDc"_q },
		.title = tr::ityc_SettingsShowID(),
		.st = &st::settingsButtonNoIcon,
		.label = std::move(currentVal),
		.onClick = [=] {
			controller->show(Box(
				[=](not_null<Ui::GenericBox*> box) {
					const auto save = [=](int index) {
						AyuSettings::getInstance().setShowPeerId(
							static_cast<PeerIdDisplay>(index));
					};
					SingleChoiceBox(box, {
						.title = tr::ityc_SettingsShowID(),
						.options = options,
						.initialSelection = static_cast<int>(settings->showPeerId()),
						.callback = save,
					});
				}));
		},
	});
}

void BuildQoLToggles(SectionBuilder &builder, AyuSectionBuilder &ayu) {
	auto *settings = &AyuSettings::getInstance();

	BuildTranslator(builder, ayu);
	ayu.addSectionDivider();

	builder.addSubsectionTitle(tr::ityc_CategoryGeneral());

	const auto controller = builder.controller();
	ayu.addToggle({
		.id = u"ityc/disableStories"_q,
		.altIds = { u"ityc/hideStories"_q },
		.title = tr::ityc_DisableStories(),
		.getter = [=] { return settings->disableStories(); },
		.setter = [=](bool enabled) {
			AyuSettings::getInstance().setDisableStories(enabled);
			ShowRestartPrompt(controller);
		},
	});

	ayu.addSettingToggle({
		.id = u"ityc/disableOpenLinkWarning"_q,
		.title = tr::ityc_DisableOpenLinkWarning(),
		.getter = &AyuSettings::disableOpenLinkWarning,
		.setter = &AyuSettings::setDisableOpenLinkWarning,
	});

	ayu.addCollapsibleToggle({
		.id = u"ityc/similarChannels"_q,
		.title = tr::ityc_DisableSimilarChannels(),
		.checkboxes = {
			NestedEntry{
				tr::ityc_CollapseSimilarChannels(tr::now),
				[] { return AyuSettings::getInstance().collapseSimilarChannels(); },
				[](bool v) { AyuSettings::getInstance().setCollapseSimilarChannels(v); }
			},
			NestedEntry{
				tr::ityc_HideSimilarChannelsTab(tr::now),
				[] { return AyuSettings::getInstance().hideSimilarChannels(); },
				[](bool v) { AyuSettings::getInstance().setHideSimilarChannels(v); }
			}
		},
		.toggledWhenAll = true,
	});

	ayu.addSettingToggle({
		.id = u"ityc/disableNotificationsDelay"_q,
		.title = tr::ityc_DisableNotificationsDelay(),
		.getter = &AyuSettings::disableNotificationsDelay,
		.setter = &AyuSettings::setDisableNotificationsDelay,
	});

	ayu.addSectionDivider();

	const auto zalgoButton = builder.addButton({
		.id = u"ityc/filterZalgo"_q,
		.title = tr::ityc_FilterZalgo(),
		.st = &st::settingsButtonNoIcon,
		.toggled = rpl::single(settings->filterZalgo()),
	});
	if (zalgoButton) {
		zalgoButton->toggledValue(
		) | rpl::filter(
			[=](bool enabled) {
				return (enabled != settings->filterZalgo());
			}
		) | on_next(
			[=](bool enabled) {
				AyuSettings::getInstance().setFilterZalgo(enabled);
				ShowRestartPrompt(controller);
			},
			zalgoButton->lifetime());
		ayu.addBetaBadge(zalgoButton);
	}

	ayu.addSettingToggle({
		.id = u"ityc/improveLinkPreviews"_q,
		.title = tr::ityc_ImproveLinkPreviews(),
		.getter = &AyuSettings::improveLinkPreviews,
		.setter = &AyuSettings::setImproveLinkPreviews,
	});
	ayu.addCollapsibleToggle({
		.id = u"ityc/confirmations"_q,
		.title = tr::ityc_ConfirmationsTitle(),
		.checkboxes = {
			NestedEntry{
				tr::ityc_StickerConfirmation(tr::now),
				[] { return AyuSettings::getInstance().stickerConfirmation(); },
				[](bool v) { AyuSettings::getInstance().setStickerConfirmation(v); }
			},
			NestedEntry{
				tr::ityc_GIFConfirmation(tr::now),
				[] { return AyuSettings::getInstance().gifConfirmation(); },
				[](bool v) { AyuSettings::getInstance().setGifConfirmation(v); }
			},
			NestedEntry{
				tr::ityc_VoiceConfirmation(tr::now),
				[] { return AyuSettings::getInstance().voiceConfirmation(); },
				[](bool v) { AyuSettings::getInstance().setVoiceConfirmation(v); }
			},
			NestedEntry{
				tr::ityc_RoundConfirmation(tr::now),
				[] { return AyuSettings::getInstance().roundConfirmation(); },
				[](bool v) { AyuSettings::getInstance().setRoundConfirmation(v); }
			}
		},
		.toggledWhenAll = false,
	});
	ayu.addSettingToggle({
		.id = u"ityc/showMessageSeconds"_q,
		.altIds = { u"ityc/formatTimeWithSeconds"_q },
		.title = tr::ityc_SettingsShowMessageSeconds(),
		.getter = &AyuSettings::showMessageSeconds,
		.setter = &AyuSettings::setShowMessageSeconds,
	});

	BuildShowPeerId(builder);

	ayu.addSectionDivider();

	builder.addSubsectionTitle(rpl::single(QString("Webview")));

	ayu.addSettingToggle({
		.id = u"ityc/spoofWebviewAsAndroid"_q,
		.title = tr::ityc_SettingsSpoofWebviewAsAndroid(),
		.getter = &AyuSettings::spoofWebviewAsAndroid,
		.setter = &AyuSettings::setSpoofWebviewAsAndroid,
	});

	ayu.addCollapsibleToggle({
		.id = u"ityc/biggerWindow"_q,
		.title = tr::ityc_SettingsBiggerWindow(),
		.checkboxes = {
			NestedEntry{
				tr::ityc_SettingsIncreaseWebviewHeight(tr::now),
				[] { return AyuSettings::getInstance().increaseWebviewHeight(); },
				[](bool v) { AyuSettings::getInstance().setIncreaseWebviewHeight(v); }
			},
			NestedEntry{
				tr::ityc_SettingsIncreaseWebviewWidth(tr::now),
				[] { return AyuSettings::getInstance().increaseWebviewWidth(); },
				[](bool v) { AyuSettings::getInstance().setIncreaseWebviewWidth(v); }
			}
		},
		.toggledWhenAll = false,
	});
}

const auto kMeta = BuildHelper({
	.id = AyuGeneral::Id(),
	.parentId = AyuMain::Id(),
	.title = &tr::ityc_CategoryGeneral,
	.icon = &st::menuIconShowAll,
}, [](SectionBuilder &builder) {
	auto ayu = AyuSectionBuilder(builder);

	builder.addSkip();
	BuildQoLToggles(builder, ayu);
	builder.addSkip();
});

} // namespace

rpl::producer<QString> AyuGeneral::title() {
	return tr::ityc_CategoryGeneral();
}

AyuGeneral::AyuGeneral(
	QWidget *parent,
	not_null<Window::SessionController*> controller)
: Section(parent, controller) {
	setupContent();
}

void AyuGeneral::setupContent() {
	const auto content = Ui::CreateChild<Ui::VerticalLayout>(this);
	build(content, kMeta.build);
	Ui::ResizeFitChild(this, content);
}

Type AyuGeneralId() {
	return AyuGeneral::Id();
}

} // namespace Settings
