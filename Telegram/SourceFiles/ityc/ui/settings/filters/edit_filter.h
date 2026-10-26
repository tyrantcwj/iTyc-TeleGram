// This is the source code of iTycGram for Desktop.
//
// We do not and cannot prevent the use of our code,
// but be respectful and credit the original author.
//
// Copyright @Radolyn, 2026
#pragma once

#include "ityc/data/entities.h"
#include "settings/settings_common_session.h"

#include "boxes/premium_limits_box.h"
#include "base/unixtime.h"

class BoxContent;

namespace Window {
class Controller;
class SessionController;
} // namespace Window

namespace Settings {

object_ptr<Ui::GenericBox> RegexEditBox(RegexFilter *filter,
										const Fn<void(RegexFilter)> &onDone,
										std::optional<long long> dialogId = std::nullopt,
										bool showToast = false);

// Saves a case-insensitive literal keyword filter and turns filters on.
void SaveLiteralFilter(const QString &phrase, std::optional<long long> dialogId);

// Batch version of SaveLiteralFilter: one phrase per entry, duplicates
// (including ones already saved for the same dialog) are skipped.
// Returns how many new filters were queued.
int SaveLiteralFilters(
	const std::vector<QString> &phrases,
	std::optional<long long> dialogId,
	bool showToast = true);

object_ptr<Ui::GenericBox> BulkLiteralFilterBox(
	std::optional<long long> dialogId = std::nullopt);
} // namespace Settings