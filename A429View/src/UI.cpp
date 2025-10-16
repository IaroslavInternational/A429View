#include "UI.hpp"

#include "CoreLog.hpp"
#include "Core/lib.hpp"
#include <imgui_internal.h>

#include <chrono>
#include <sstream>
#include <fstream>
#include <filesystem>
#include <numeric>

#pragma execution_character_set("utf-8")  // Для отображения на русском языке

using namespace std::literals::chrono_literals;

static long long avg(std::vector<long long> const& v)
{
	return v.empty() ? 0.0 : std::accumulate(v.begin(), v.end(), 0) / v.size();
}

UI::UI()
{
	ImGui::GetStyle().WindowBorderSize = 0.0f;
	ImGui::GetStyle().TabBorderSize    = 1.0f;
	ImGui::GetStyle().TabBarBorderSize = 1.0f;
	ImGui::GetStyle().FrameRounding    = 8.0f;
	ImGui::GetStyle().FramePadding.x   = 10.0f;
	ImGui::GetStyle().GrabRounding     = 5.0f;

	ImVec4* colors = ImGui::GetStyle().Colors;
	colors[ImGuiCol_WindowBg]       = ImVec4(0.10f, 0.10f, 0.10f, 1.00f);
	colors[ImGuiCol_FrameBg]        = ImVec4(0.07f, 0.07f, 0.07f, 0.54f);
	colors[ImGuiCol_TitleBg]        = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
	colors[ImGuiCol_TitleBgActive]  = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
	colors[ImGuiCol_MenuBarBg]      = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
	colors[ImGuiCol_TabSelected]    = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
	colors[ImGuiCol_TabHovered]     = ImVec4(0.25f, 0.25f, 0.25f, 0.80f);
	colors[ImGuiCol_Tab]            = ImVec4(0.02f, 0.02f, 0.02f, 0.86f);
	colors[ImGuiCol_Button]         = ImVec4(0.55f, 0.47f, 0.03f, 0.91f);
	colors[ImGuiCol_ButtonHovered]  = ImVec4(0.05f, 0.07f, 0.09f, 1.00f);
	colors[ImGuiCol_FrameBgHovered] = ImVec4(0.17f, 0.18f, 0.19f, 0.40f);
	colors[ImGuiCol_FrameBgActive]  = ImVec4(0.24f, 0.26f, 0.29f, 0.40f);
	colors[ImGuiCol_CheckMark]      = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
	colors[ImGuiCol_Button]         = ImVec4(0.34f, 0.06f, 0.06f, 0.91f);
	colors[ImGuiCol_ButtonActive]   = ImVec4(0.09f, 0.11f, 0.13f, 1.00f);
	colors[ImGuiCol_Header]         = ImVec4(0.42f, 0.13f, 0.13f, 0.31f);
	colors[ImGuiCol_HeaderHovered]  = ImVec4(0.22f, 0.05f, 0.05f, 0.80f);
	colors[ImGuiCol_HeaderActive]   = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
	colors[ImGuiCol_PopupBg] = ImVec4(0.15f, 0.15f, 0.15f, 1.00f);

	LOG_H("UI");
	LOG("Colors are set\n");

	LOG_END();

	/*rk_words[0].word = 0xF;
	rk_words[1].word = 0xF;
	rk_words[2].word = 0xF;
	rk_words[3].word = 0xF;
	rk_words[4].word = 0xF;
	rk_words[5].word = 0xF;
	rx_labels[0][0244].word.value = 0244;*/
}

void UI::Render(float dt)
{
	//ImGui::ShowDemoWindow();

	SetPanelSizeAndPosition(0, 0.25f, 1.0f, 0.0f, 0.0f);
	ShowLeftPanel();

	SetPanelSizeAndPosition(0, 0.75f, 0.95f, 0.25f, 0.0f);
	ShowTable();
}

void UI::ShowLeftPanel()
{
	if (ImGui::Begin("MainBar", NULL, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoBringToFrontOnFocus))
	{
		if (ImGui::BeginTabBar("Main bar"))
		{
			if (ImGui::BeginTabItem("Подключение"))
			{
				auto ports = getAvailablePorts();
				std::vector<std::string> names;

				for (auto& p : ports)
				{
					names.emplace_back(std::string("COM") + std::to_string(p) + '\0');
				}

				static int32_t item_current = 0;
				ImGui::Combo("Порты", &item_current,
					[](void* vec, int idx, const char** out_text) 
					{
						std::vector<std::string>* vector = reinterpret_cast<std::vector<std::string>*>(vec);
							if (idx < 0 || idx >= vector->size()) 
								return false;
						*out_text = vector->at(idx).c_str();
						return true;
					}, reinterpret_cast<void*>(&names), names.size());

				if (fault.bits.time_out)
				{
					ImGui::Text(("Не удалось подключиться к " + names[item_current]).c_str());
				}
				else
				{
					ImGui::Text(("Подключено к " + port.GetName()).c_str());
				}

				if (!port.IsOpen())
				{
					if (names.size() != 0)
					{
						if (ImGui::Button("Подключить"))
						{
							ConnectionThread = std::async(std::launch::async, &UI::TryConnection, this, names[item_current]);
						}
					}
					else
					{
						ImGui::Text("Нет доступных портов");
					}
				}
				else
				{
					if (ImGui::Button("Закрыть"))
					{
						ConnectionThread = std::async(std::launch::async, &UI::CloseConnection, this);
					}
				}
				
				ImGui::EndTabItem();
			}
		}

		ImGui::EndTabBar();
	}

	ImGui::End();
}

static void RenderBufItems(const std::string& name, const a429_buf_t& buffer)
{
	std::ostringstream oss;

	for (auto& [channel, flow] : buffer)
	{
		for (auto& [label, frame] : flow)
		{
			ImGui::TableNextRow();

			// Создаем уникальный ID для строки
			ImGui::PushID(static_cast<int>(channel * 1000 + label));

			ImGui::TableSetColumnIndex(0);
			oss << name << channel + 1;
			ImGui::Text("%s", oss.str().c_str());
			oss.str(""); oss.clear();

			ImGui::TableSetColumnIndex(1);
			oss << std::oct << label;
			ImGui::Text("%s", oss.str().c_str());
			oss.str(""); oss.clear();

			ImGui::TableSetColumnIndex(2);
			oss << "0x" << std::hex << std::uppercase << frame.word.bits.sdi;
			ImGui::Text("%s", oss.str().c_str());
			oss.str(""); oss.clear();

			ImGui::TableSetColumnIndex(3);
			oss << "0x" << std::hex << std::uppercase << frame.word.bits.data;
			ImGui::Text("%s", oss.str().c_str());
			oss.str(""); oss.clear();

			ImGui::TableSetColumnIndex(4);
			oss << "0x" << std::hex << std::uppercase << frame.word.bits.ssm;
			ImGui::Text("%s", oss.str().c_str());
			oss.str(""); oss.clear();

			ImGui::TableSetColumnIndex(5);
			oss << frame.word.bits.parity;
			ImGui::Text("%s", oss.str().c_str());
			oss.str(""); oss.clear();

			ImGui::TableSetColumnIndex(6);
			oss << std::dec << frame.delta << " ms";
			ImGui::Text("%s", oss.str().c_str());
			oss.str(""); oss.clear();

			ImGui::TableSetColumnIndex(0);
			ImGui::Selectable("##row_selectable", false, ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowOverlap);

			// Открываем попап при правом клике на строку
			if (ImGui::BeginPopupContextItem("##row_context"))
			{
				constexpr ImGuiTableFlags flags = ImGuiTableFlags_BordersOuter | ImGuiTableFlags_BordersV |
					ImGuiTableFlags_RowBg | ImGuiTableFlags_Resizable;

				// ТАБЛИЦА БИТОВ
				if (ImGui::BeginTable("##bit_table", 32, flags, ImVec2(-1, 0)))
				{
					// Устанавливаем одинаковую ширину для всех колонок
					for (int i = 0; i < 32; i++)
					{
						ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthFixed, 15.0f); // Фиксированная ширина
					}

					// Заголовок - номера битов 
					ImGui::TableNextRow();
					for (int i = 31; i >= 0; i--)
					{
						ImGui::TableSetColumnIndex(31 - i);
						ImGui::Text("%d", i);
					}

					// Значения битов
					ImGui::TableNextRow();
					for (int i = 31; i >= 0; i--)
					{
						ImGui::TableSetColumnIndex(31 - i);
						bool bit_value = (frame.word.value >> i) & 1;

						// Цвет для разных типов битов
						if (i == 31)
						{
							ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1, 0.2f, 0.2f, 1)); // Parity - красный
						}
						else if (i >= 29)
						{
							ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1, 0.8f, 0.2f, 1)); // SSM - оранжевый
						}
						else if (i >= 10)
						{
							ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.2f, 1, 0.2f, 1)); // Data - зеленый
						}
						else if (i >= 8)
						{
							ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.2f, 0.8f, 1, 1)); // SDI - голубой
						}
						else
						{
							ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1, 1, 1, 1)); // Label - белый
						}

						ImGui::Text("%d", bit_value);
						ImGui::PopStyleColor();
					}

					ImGui::EndTable();
				}

				ImGui::EndPopup();
			}

			ImGui::PopID();
		}
	}
}

static void RenderBufRK(const std::string& name, const std::vector<std::string> bit_names, rk_flow_t flow)
{
	std::ostringstream oss;

	static const ImGuiTableFlags flags = ImGuiTableFlags_BordersOuter | ImGuiTableFlags_BordersV |
		ImGuiTableFlags_RowBg | ImGuiTableFlags_Reorderable;

	if (ImGui::TreeNode(name.c_str()))
	{
		if (ImGui::BeginTable((name + "_t").c_str(), 2, flags, ImVec2(ImGui::GetCurrentWindow()->Size.x / 2, 0)))
		{
			oss << "Value (dt: " << flow.delta << " ms)";

			ImGui::TableSetupColumn("Parameter", ImGuiTableColumnFlags_WidthFixed, 250.0f);
			ImGui::TableSetupColumn(oss.str().c_str(), ImGuiTableColumnFlags_WidthStretch);
			ImGui::TableHeadersRow();

			for (int i = 0; i < bit_names.size(); i++)
			{
				ImGui::TableNextRow();

				if (i == 0)
				{
					ImGui::TableSetColumnIndex(0);
					ImGui::TextColored({0.7f, 0.2f, 0.2f, 1.0f}, "Word");

					ImGui::TableSetColumnIndex(1);
					ImGui::TextColored({ 0.7f, 0.2f, 0.2f, 1.0f }, "0x%X", flow.word);

					continue;
				}

				ImGui::TableSetColumnIndex(0);
				ImGui::Text("%s", bit_names[i].c_str());

				ImGui::TableSetColumnIndex(1);
				ImGui::Text("%d", (flow.word >> i) & 1);
			}

			ImGui::EndTable();
		}

		ImGui::TreePop();
	}
}

void UI::ShowTable()
{
	std::ostringstream oss;

	ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(17.0f / 255.0f, 17.0f / 255.0f, 17.0f / 255.0f, 1.00f));
	if (ImGui::Begin("Data Metrics", NULL, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoBringToFrontOnFocus))
	{
		static const ImGuiTableFlags flags = ImGuiTableFlags_BordersOuter | ImGuiTableFlags_BordersV |
			ImGuiTableFlags_RowBg | ImGuiTableFlags_Reorderable;

		if (ImGui::BeginTabBar("Buffers"))
		{
			if (ImGui::BeginTabItem("RX Buffer"))
			{
				ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(8, 4));
				ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(4, 2));

				if (ImGui::BeginTable("##table_rx", 7, flags, ImVec2(-1, 0)))
				{
					ImGui::TableSetupColumn("Channel", ImGuiTableColumnFlags_WidthFixed, 75.0f);
					ImGui::TableSetupColumn("Label",   ImGuiTableColumnFlags_WidthFixed, 75.0f);
					ImGui::TableSetupColumn("SDI",     ImGuiTableColumnFlags_WidthFixed, 50.0f);
					ImGui::TableSetupColumn("Data",    ImGuiTableColumnFlags_WidthFixed, 100.0f);
					ImGui::TableSetupColumn("SSM",     ImGuiTableColumnFlags_WidthFixed, 50.0f);
					ImGui::TableSetupColumn("Parity",  ImGuiTableColumnFlags_WidthFixed, 50.0f);
					ImGui::TableSetupColumn("Period");
					ImGui::TableHeadersRow();

					RenderBufItems("RX", rx_labels);

					ImGui::EndTable();
				}

				ImGui::PopStyleVar(2);				
				ImGui::EndTabItem();
			}

			if (ImGui::BeginTabItem("TX Buffer"))
			{
				ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(8, 4));
				ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(4, 2));

				if (ImGui::BeginTable("##table_tx", 7, flags, ImVec2(-1, 0)))
				{
					ImGui::TableSetupColumn("Channel", ImGuiTableColumnFlags_WidthFixed, 75.0f);
					ImGui::TableSetupColumn("Label",   ImGuiTableColumnFlags_WidthFixed, 75.0f);
					ImGui::TableSetupColumn("SDI",     ImGuiTableColumnFlags_WidthFixed, 50.0f);
					ImGui::TableSetupColumn("Data",    ImGuiTableColumnFlags_WidthFixed, 100.0f);
					ImGui::TableSetupColumn("SSM",     ImGuiTableColumnFlags_WidthFixed, 50.0f);
					ImGui::TableSetupColumn("Parity",  ImGuiTableColumnFlags_WidthFixed, 50.0f);
					ImGui::TableSetupColumn("Period");
					ImGui::TableHeadersRow();

					RenderBufItems("TX", tx_labels);

					ImGui::EndTable();
				}

				ImGui::PopStyleVar(2);
				ImGui::EndTabItem();
			}
		
			if (ImGui::BeginTabItem("RK Buffer"))
			{

				static std::vector<std::string> PANEL_BITS = 
				{
					"lh_ac_aux_pump_cmd", "lh_ac_main_pump_cmd", "lh_addt_pump_cmd",
					"rh_ac_aux_pump_cmd", "rh_ac_main_pump_cmd", "rh_addt_pump_cmd",
					"xfeed_on_ch1", "xfeed_on_ch2", "xfeed_off_ch1", "xfeed_off_ch2"
				};

				static std::vector<std::string> VALVES_BITS = 
				{
					"apu_vlv_closed",        // Бит 0
					"apu_vlv_open",          // Бит 1
					"ct_rv_open",            // Бит 2
					"ct_rv_closed",          // Бит 3
					"dv_open",               // Бит 4
					"dv_closed",             // Бит 5
					"l_eng_vlv_closed",      // Бит 6
					"l_eng_vlv_open",        // Бит 7
					"lh_rv_open",            // Бит 8
					"lh_rv_closed",          // Бит 9
					"r_eng_vlv_closed",      // Бит 10
					"r_eng_vlv_open",        // Бит 11
					"rh_rv_open",            // Бит 12
					"rh_rv_closed",          // Бит 13
					"xfeed_vlv_closed",      // Бит 14
					"xfeed_vlv_open"         // Бит 15
				};

				// PUMPS_IN_REG_t - регистр состояния топливных насосов
				static std::vector<std::string> PUMPS_BITS = 
				{
					"lh_addt_pump_normal_press",    // Бит 0
					"lh_addt_pump_low_press",       // Бит 1
					"rh_addt_pump_normal_press",    // Бит 2
					"rh_addt_pump_low_press",       // Бит 3
					"lh_ac_main_pump_normal_press", // Бит 4
					"lh_ac_main_pump_low_press",    // Бит 5
					"lh_ac_aux_pump_normal_press",  // Бит 6
					"lh_ac_aux_pump_low_press",     // Бит 7
					"rh_ac_main_pump_normal_press", // Бит 8
					"rh_ac_main_pump_low_press",    // Бит 9
					"rh_ac_aux_pump_normal_press",  // Бит 10
					"rh_ac_aux_pump_low_press",     // Бит 11
					"lh_addt_pump_powered",         // Бит 12
					"rh_addt_pump_powered",         // Бит 13
					"lh_ac_main_pump_powered",      // Бит 14
					"lh_ac_aux_pump_powered",       // Бит 15
					"rh_ac_main_pump_powered",      // Бит 16
					"rh_ac_aux_pump_powered",       // Бит 17
					"lh_eng_low_press",             // Бит 18
					"lh_eng_norm_press",            // Бит 19
					"rh_eng_low_press",             // Бит 20
					"rh_eng_norm_press",            // Бит 21
					"apu_low_press",                // Бит 22
					"apu_norm_press"                // Бит 23
				};

				// ENG_FIRE_IN_REG_t - регистр пусков МСУ и пожарной защиты
				static std::vector<std::string> ENG_FIRE_BITS = 
				{
					"l_eng_off_ch1",  // Бит 0
					"l_eng_off_ch2",  // Бит 1
					"l_eng_on_ch1",   // Бит 2
					"l_eng_on_ch2",   // Бит 3
					"r_eng_off_ch1",  // Бит 4
					"r_eng_off_ch2",  // Бит 5
					"r_eng_on_ch1",   // Бит 6
					"r_eng_on_ch2"    // Бит 7
				};

				// APU_IN_REG_t - регистр пусков ВСУ и пожарной защиты
				static std::vector<std::string> APU_BITS = 
				{
					"apu_fire_ch1",  // Бит 0
					"apu_fire_ch2",  // Бит 1
					"apu_off_ch1",   // Бит 2
					"apu_off_ch2",   // Бит 3
					"apu_on_ch1",    // Бит 4
					"apu_on_ch2"     // Бит 5
				};

				// VALVES_OUT_REG_t - регистр управления запорными кранами
				static std::vector<std::string> VALVES_OUT_BITS = 
				{
					"apu_vlv_close_cmd_ch1",      // Бит 0
					"apu_vlv_close_cmd_ch2",      // Бит 1
					"apu_vlv_open_cmd_ch1",       // Бит 2
					"apu_vlv_open_cmd_ch2",       // Бит 3
					"ct_refuel_vlv_open_cmd_ch1", // Бит 4
					"ct_refuel_vlv_open_cmd_ch2", // Бит 5
					"dv_closed_cmd",              // Бит 6
					"dv_open_cmd",                // Бит 7
					"lh_refuel_vlv_open_cmd_ch1", // Бит 8
					"lh_refuel_vlv_open_cmd_ch2", // Бит 9
					"rh_refuel_vlv_open_cmd_ch1", // Бит 10
					"rh_refuel_vlv_open_cmd_ch2", // Бит 11
					"xfeed_vlv_close_cmd_ch1",    // Бит 12
					"xfeed_vlv_close_cmd_ch2",    // Бит 13
					"xfeed_vlv_open_cmd_ch1",     // Бит 14
					"xfeed_vlv_open_cmd_ch2"      // Бит 15
				};

				if (rk_words.size() > 0)
				{
					RenderBufRK("Panel",      PANEL_BITS,      rk_words[0]);
					RenderBufRK("Valves",     VALVES_BITS,     rk_words[1]);
					RenderBufRK("Pumps",      PUMPS_BITS,      rk_words[2]);
					RenderBufRK("Eng fire",   ENG_FIRE_BITS,   rk_words[3]);
					RenderBufRK("APU",        APU_BITS,        rk_words[4]);
					RenderBufRK("Valves out", VALVES_OUT_BITS, rk_words[5]);
				}

				ImGui::EndTabItem();
			}
		}

		ImGui::EndTabBar();
		ImGui::PopStyleColor();
	}
	
	ImGui::End();
}

void UI::SetPanelSizeAndPosition(int corner, float width, float height, float x_offset, float y_offset)
{
	ImGuiIO& io = ImGui::GetIO();

	ImVec2 DispSize = io.DisplaySize;

	float PanelW = round(DispSize.x * width);
	float PanelH = DispSize.y * height;

	ImVec2 PanelSize = ImVec2(PanelW, PanelH);

	// Учитываем высоту меню при позиционировании
	ImVec2 PanelPos = ImVec2(
		(corner & 1) ? DispSize.x + round(DispSize.x * x_offset) : round(DispSize.x * x_offset),
		(corner & 2) ? DispSize.y + DispSize.y * y_offset
		: DispSize.y * y_offset
	);

	ImVec2 PanelPivot = ImVec2(
		(corner & 1) ? 1.0f : 0.0f,
		(corner & 2) ? 1.0f : 0.0f
	);

	ImGui::SetNextWindowPos(PanelPos, ImGuiCond_Always, PanelPivot);
	ImGui::SetNextWindowSize(PanelSize);
}

std::list<int> UI::getAvailablePorts()
{
	wchar_t lpTargetPath[5000];
	std::list<int> portList;

	for (uint16_t i = 0; i < 255; i++)
	{
		std::wstring str = L"COM" + std::to_wstring(i);
		DWORD res = QueryDosDevice(str.c_str(), lpTargetPath, 5000);

		if (res != 0) 
		{
			portList.push_back(i);
		}
	}

	return portList;
}

void UI::TryConnection(const std::string& name)
{
	if (!port.Open(name, 256000U, 8, 1))
	{
		LOG("TRY FAILED");
		port.Close();
		return;
	}
	
	ThreadsAllowed = true;
	
	RxThread = std::async(std::launch::async, &UI::ReceiveData, this);
}

void UI::CloseConnection()
{
	ThreadsAllowed = false;

	// Каждые 50мс ожидаем завершения потоков и закрываем порт
	while (!lib::IsThreadTerminated(RxThread))
	{
		std::this_thread::sleep_for(50ms);
	}

	port.Close();	
	RxThread = std::future<void>();

	rx_labels.clear();
	tx_labels.clear();
	rk_words.clear();
}

void UI::ReceiveData()
{
	buffer_t buf;

	uint32_t empty_reads = 0;
	constexpr uint32_t MAX_EMPTY_READS = 100; // После 100 пустых чтений - очистка

	while (ThreadsAllowed)
	{
		if (port.RxData(&buf)) 
		{
			DataProc(&buf);
			empty_reads = 0;  // Сброс счетчика при успешном чтении
		}
		else 
		{
			empty_reads++;

			if (empty_reads > MAX_EMPTY_READS) 
			{
				port.ClearBuffer();  // Периодическая очистка
				empty_reads = 0;
			}
		}

		std::this_thread::sleep_for(1ms);
	}
}

void UI::DataProc(buffer_t* buf)
{
	uint32_t    word;
	uint32_t    channel;
	a429_flow_t a429;
	a429_buf_t* current_buffer = nullptr;

	std::string str{ buf->begin(), buf->end() };
	int pos;

	std::stringstream ss;

	std::chrono::steady_clock::time_point stamp = std::chrono::steady_clock::now();

	while (str.size() > 0 && ThreadsAllowed)
	{
		pos = str.find_first_of("|_r"); // Ищем любой из разделителей

		if (pos == std::string::npos) // Если разделителей нет - выходим
		{
			break;
		}

		if (str[pos] == '|')
		{
			current_buffer = &rx_labels;
		}
		else if (str[pos] == '_')
		{
			current_buffer = &tx_labels;
		}

		if ((current_buffer != nullptr || str[pos] == 'r') && pos > 0) // Добавил проверку pos > 0
		{
			ss << std::string(str.begin(), str.begin() + pos - 1);
			ss >> word;

			channel = std::stoi(std::string(str.begin() + pos - 1, str.begin() + pos));
			
			if (channel == 5 && str[pos] == 'r')
			{
				find_s = word;
			}

			mtx.lock();
			if (str[pos] == 'r')
			{
				rk_words[channel].word = word;

				auto& data = rk_words[channel];
				if (data.stamp.time_since_epoch().count() > 0) // Проверка инициализации
				{
					data.delta_buf.push_back(std::chrono::duration_cast<std::chrono::milliseconds>(stamp - data.stamp).count());
				}
				data.stamp = stamp;

				if (data.delta_buf.size() > 2)
				{
					data.delta = avg(data.delta_buf);
					data.delta_buf.erase(data.delta_buf.begin());
				}
			}
			else
			{
				a429.word.value = word;

				(*current_buffer)[channel][a429.word.bits.label].word = a429.word;

				auto& data = (*current_buffer)[channel][a429.word.bits.label];
				if (data.stamp.time_since_epoch().count() > 0) // Проверка инициализации
				{
					data.delta_buf.push_back(std::chrono::duration_cast<std::chrono::milliseconds>(stamp - data.stamp).count());
				}
				data.stamp = stamp;

				if (data.delta_buf.size() > 2)
				{
					data.delta = avg(data.delta_buf);
					data.delta_buf.erase(data.delta_buf.begin());
				}
			}

			str = std::string(str.begin() + pos + 1, str.end());
			mtx.unlock();

			ss.str("");
			ss.clear();
		}
		else
		{
			break; 
		}
	}
}
