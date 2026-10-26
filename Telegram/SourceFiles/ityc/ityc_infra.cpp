// This is the source code of iTycGram for Desktop.
//
// We do not and cannot prevent the use of our code,
// but be respectful and credit the original author.
//
// Copyright @Radolyn, 2026
#include "ityc/ityc_infra.h"

#include "ityc/ityc_lang.h"
#include "ityc/ityc_settings.h"
#include "ityc/ityc_ui_settings.h"
#include "ityc/ityc_worker.h"
#include "ityc/data/ityc_database.h"
#include "ityc/ui/ityc_logo.h"
#include "features/translator/ityc_translator.h"
#include "lang/lang_instance.h"
#include "rpl/lifetime.h"
#include "ui/chat/chat_style_radius.h"
#include "utils/rc_manager.h"

#ifdef Q_OS_WIN
#include "ityc/utils/windows_utils.h"
#endif

namespace AyuInfra {

void initLang() {
	// Language is applied after startup on Windows, so the first read is often
	// empty. Retry when the active language id changes, otherwise iTycGram
	// strings stay on the English defaults.
	static const auto lifetime = std::make_shared<rpl::lifetime>();
	const auto apply = [] {
		const auto id = Lang::GetInstance().id();
		const auto baseId = Lang::GetInstance().baseId();
		if (id.isEmpty()) {
			LOG(("Language is not loaded"));
			return;
		}
		AyuLanguage::init();
		if (const auto instance = AyuLanguage::currentInstance()) {
			instance->fetchLanguage(id, baseId);
		}
	};
	apply();
	Lang::GetInstance().idChanges(
	) | rpl::on_next([=](const QString &) {
		apply();
	}, *lifetime);
}

void initUiSettings() {
	const auto &settings = AyuSettings::getInstance();

	AyuUiSettings::setMonoFont(settings.monoFont());
	AyuUiSettings::setWideMultiplier(settings.wideMultiplier());
	AyuUiSettings::setMaterialSwitches(settings.materialSwitches());
	AyuUiSettings::setAvatarCorners(settings.avatarCorners());
	Ui::SetAppliedBubbleRadius(settings.messageBubbleRadius());
}

void initDatabase() {
	AyuDatabase::initialize();
}

void initWorker() {
	AyuWorker::initialize();
}

void initRCManager() {
	RCManager::getInstance().start();
}

void initTranslator() {
	Ayu::Translator::TranslateManager::init();
}

void initIcon() {
#ifdef Q_OS_WIN
	AyuAssets::loadAppIco();
	reloadAppIconFromTaskBar();
#endif
}

void init() {
	initLang();
	initDatabase();
	initUiSettings();
	initIcon();
	initWorker();
	initRCManager();
	initTranslator();
}

}
