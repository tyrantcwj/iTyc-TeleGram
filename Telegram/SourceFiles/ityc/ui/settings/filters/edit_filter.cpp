// This is the source code of iTycGram for Desktop.
//
// We do not and cannot prevent the use of our code,
// but be respectful and credit the original author.
//
// Copyright @Radolyn, 2026
#include "ityc/ui/settings/filters/edit_filter.h"

#include "lang_auto.h"
#include "ityc/ityc_settings.h"
#include "ityc/data/ityc_database.h"
#include "ityc/features/filters/filters_cache_controller.h"
#include "ityc/ui/toasts.h"
#include "base/event_filter.h"
#include "base/platform/base_platform_info.h"
#include "boxes/delete_messages_box.h"
#include "core/mime_type.h"
#include "lang/lang_text_entity.h"
#include "media/audio/media_audio.h"
#include "media/view/media_view_pip.h"
#include "styles/style_boxes.h"
#include "styles/style_layers.h"
#include "styles/style_settings.h"
#include "styles/style_window.h"
#include "ui/ui_utility.h"
#include "ui/boxes/confirm_box.h"
#include "ui/text/text.h"
#include "ui/toast/toast.h"
#include "ui/widgets/checkbox.h"
#include "ui/widgets/labels.h"
#include "ui/widgets/fields/input_field.h"
#include "ui/wrap/slide_wrap.h"
#include "ui/wrap/vertical_layout.h"

namespace Settings {

std::vector<char> generate_uuid_bytes() {
	// stolen somewhere from Internet
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_int_distribution<uint32_t> dist;

	std::vector<uint8_t> bytes(16);
	for (int i = 0; i < 16; i += 4) {
		uint32_t random_chunk = dist(gen);
		bytes[i] = random_chunk & 0xFF;
		bytes[i + 1] = (random_chunk >> 8) & 0xFF;
		bytes[i + 2] = (random_chunk >> 16) & 0xFF;
		bytes[i + 3] = (random_chunk >> 24) & 0xFF;
	}
	bytes[6] = (bytes[6] & 0x0F) | 0x40;
	bytes[8] = (bytes[8] & 0x3F) | 0x80;

	return std::vector<char>(bytes.begin(), bytes.end());
}

bool validateRegex(const icu::UnicodeString& pattern, std::string& errorMsg) {
	UErrorCode status = U_ZERO_ERROR;
	UParseError parseError;

	icu::RegexPattern* regexPattern = icu::RegexPattern::compile(
		pattern,
		0, // flags
		parseError,
		status
	);

	if (U_FAILURE(status)) {
		auto errorCodeNormalized = std::string(u_errorName(status));
		errorCodeNormalized = errorCodeNormalized.substr(8); // skip U_REGEX_
		std::ranges::transform(
			errorCodeNormalized,
			errorCodeNormalized.begin(),
			[](unsigned char c)
			{
				if (c == '_') {
					return std::tolower(' ');
				}
				return std::tolower(c);
			});
		errorCodeNormalized[0] = std::toupper(errorCodeNormalized[0]);
		errorMsg = errorCodeNormalized + " at " + std::to_string(parseError.offset);

		if (parseError.preContext[0] != 0 || parseError.postContext[0] != 0) {
			icu::UnicodeString pre(parseError.preContext);
			icu::UnicodeString post(parseError.postContext);
			std::string preStr, postStr;
			pre.toUTF8String(preStr);
			post.toUTF8String(postStr);
			errorMsg += " (near: '" + preStr + "' -> '" + postStr + "')";
		}

		delete regexPattern;
		return false;
	}

	delete regexPattern;
	return true;
}

not_null<Ui::SlideWrap<Ui::FlatLabel>*> AddError(
	not_null<Ui::VerticalLayout*> content,
	Ui::InputField *input) {

	std::string errorText;
	validateRegex(icu::UnicodeString::fromUTF8("("), errorText);

	const auto error = content->add(
		object_ptr<Ui::SlideWrap<Ui::FlatLabel>>(
			content,
			object_ptr<Ui::FlatLabel>(
				content,
				QString::fromStdString(errorText) + QString::fromStdString(errorText),
				st::settingLocalPasscodeError), st::settingsCheckboxPadding));
	error->hide(anim::type::instant);
	if (input) {
		input->changes() | rpl::on_next(
			[=]
			{
				error->hide(anim::type::normal);
			},
			input->lifetime());
	}
	return error;
};

void RegexEditBuilder(
	not_null<Ui::GenericBox*> box,
	RegexFilter *filter,
	const Fn<void(RegexFilter)> &onDone,
	std::optional<long long> dialogId,
	bool showToast
) {
	RegexFilter data;

	if (filter) {
		box->setTitle(showToast ? tr::ityc_RegexFiltersAdd() : tr::ityc_RegexFiltersEdit());
		data = *filter;
	} else {
		box->setTitle(tr::ityc_RegexFiltersAdd());
		data.enabled = true;
		data.caseInsensitive = true;
		data.reversed = false;
	}

	const auto regexValue = box->addRow(
		object_ptr<Ui::InputField>(
			box->verticalLayout(),
			st::windowFilterNameInput,
			Ui::InputField::Mode::MultiLine,
			tr::ityc_RegexFiltersPlaceholder()),
		st::markdownLinkFieldPadding);
	const auto errorText = AddError(box->verticalLayout(), regexValue);
	const auto enabled = box->addRow(
		object_ptr<Ui::Checkbox>(
			box,
			tr::ityc_EnableExpression(tr::now),
			data.enabled,
			st::defaultBoxCheckbox),
		st::settingsCheckboxPadding);
	const auto caseInsensitive = box->addRow(
		object_ptr<Ui::Checkbox>(
			box,
			tr::ityc_CaseInsensitiveExpression(tr::now),
			data.caseInsensitive,
			st::defaultBoxCheckbox),
		st::settingsCheckboxPadding);
	const auto reversed = box->addRow(
		object_ptr<Ui::Checkbox>(
			box,
			tr::ityc_ReversedExpression(tr::now),
			data.reversed,
			st::defaultBoxCheckbox),
		st::settingsCheckboxPadding);

	regexValue->setText(QString::fromStdString(data.text));

	auto saveAndClose = [=, id = data.id]
	{
		const auto text = regexValue->getTextWithTags().text;
		if (text.isEmpty()) {
			return;
		}

		std::string error;
		if (!validateRegex(icu::UnicodeString::fromUTF8(text.toStdString()), error)) {
			errorText->entity()->setText(QString::fromStdString(error));
			errorText->show(anim::type::normal);
			return;
		}

		RegexFilter newFilter;
		newFilter.text = regexValue->getTextWithTags().text.toStdString();
		newFilter.enabled = enabled->checked();
		newFilter.caseInsensitive = caseInsensitive->checked();
		newFilter.reversed = reversed->checked();

		if (!showToast && dialogId.has_value()) {
			newFilter.dialogId = dialogId;
		}

		if (!id.empty()) {
			newFilter.id = id;
		} else {
			newFilter.id = generate_uuid_bytes();
		}

		box->closeBox();

		crl::async([=]
		{
			AyuDatabase::addRegexFilter(newFilter);
			FiltersCacheController::rebuildCache();

			crl::on_main([=]
			{
				if (onDone) {
					onDone(newFilter);
				}
				FiltersCacheController::fireUpdate();

				if (showToast) {
					auto config = Ui::Toast::Config{
						.text = tr::ityc_RegexFilterBulletinText(
							tr::now,
							tr::rich),
						.adaptive = true,
					};
					if (dialogId.has_value()) {
						Ayu::Ui::ShowToastWithAction(
							std::move(config),
							tr::ityc_RegexFilterBulletinAction(tr::now),
							[=]() mutable {
								newFilter.dialogId = dialogId;

								AyuDatabase::updateRegexFilter(newFilter);
								FiltersCacheController::rebuildCache();
								FiltersCacheController::fireUpdate();
							});
					} else {
						Ui::Toast::Show(std::move(config));
					}
				}
			});
		});
	};

	regexValue->submits() | rpl::on_next(saveAndClose, regexValue->lifetime());
	box->addButton(tr::lng_settings_save(), saveAndClose);
	box->addButton(tr::lng_cancel(),
				   [=]
				   {
					   box->closeBox();
				   });
	box->setFocusCallback([=] {
		regexValue->setFocusFast();
	});

	errorText->entity()->resizeToWidth(box->width());
	errorText->resizeToWidth(box->width());
}

object_ptr<Ui::GenericBox> RegexEditBox(RegexFilter *filter,
										const Fn<void(RegexFilter)> &onDone,
										std::optional<long long> dialogId,
										bool showToast) {
	return Box(RegexEditBuilder, filter, onDone, dialogId, showToast);
}

QString EscapeLiteralRegex(const QString &phrase) {
	QString escaped;
	escaped.reserve(phrase.size() * 2);
	for (const auto ch : phrase) {
		switch (ch.unicode()) {
		case '\\':
		case '.':
		case '^':
		case '$':
		case '|':
		case '?':
		case '*':
		case '+':
		case '(':
		case ')':
		case '[':
		case ']':
		case '{':
		case '}':
			escaped.append('\\');
			break;
		default:
			break;
		}
		escaped.append(ch);
	}
	return escaped;
}

void SaveLiteralFilter(const QString &phrase, std::optional<long long> dialogId) {
	const auto text = phrase.trimmed();
	if (text.isEmpty()) {
		return;
	}

	auto &settings = AyuSettings::getInstance();
	if (!settings.filtersEnabled()) {
		settings.setFiltersEnabled(true);
	}
	if (!settings.filtersEnabledInChats()) {
		settings.setFiltersEnabledInChats(true);
	}

	RegexFilter filter;
	filter.id = generate_uuid_bytes();
	filter.text = EscapeLiteralRegex(text).toStdString();
	filter.enabled = true;
	filter.caseInsensitive = true;
	filter.reversed = false;
	filter.dialogId = dialogId;

	crl::async([=] {
		AyuDatabase::addRegexFilter(filter);
		FiltersCacheController::rebuildCache();
		crl::on_main([=] {
			FiltersCacheController::fireUpdate();
			auto preview = text;
			preview.replace('\n', ' ');
			preview = preview.simplified();
			if (preview.size() > 24) {
				preview = preview.left(24) + QStringLiteral("…");
			}
			Ui::Toast::Show(Ui::Toast::Config{
				.text = (dialogId.has_value()
					? u"已在此对话屏蔽「%1」"_q
					: u"已屏蔽包含「%1」的消息"_q).arg(preview),
				.adaptive = true,
			});
		});
	});
}

} // namespace Settings
