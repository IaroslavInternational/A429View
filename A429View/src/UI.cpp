#include "UI.hpp"

#include "CoreLog.hpp"
#include "Core/lib.hpp"
#include "../imgui/imgui_internal.h"

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
	colors[ImGuiCol_PopupBg]        = ImVec4(0.08f, 0.08f, 0.08f, 1.00f);

	LOG_H("UI");
	LOG("Colors are set\n");

	LOG_END();
}

void UI::Render(float dt)
{
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
					if (ImGui::Button("Подключить"))
					{				
						ConnectionThread = std::async(std::launch::async, &UI::TryConnection, this, names[item_current]);
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

void UI::ShowTable()
{
	std::ostringstream oss;

	ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(17.0f / 255.0f, 17.0f / 255.0f, 17.0f / 255.0f, 1.00f));
	if (ImGui::Begin("Data Metrics", NULL, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoBringToFrontOnFocus))
	{
		static const ImGuiTableFlags flags = ImGuiTableFlags_BordersOuter | ImGuiTableFlags_BordersV |
			ImGuiTableFlags_RowBg | ImGuiTableFlags_Resizable | ImGuiTableFlags_Reorderable;

		ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(8, 4));
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(4, 2));

		if (ImGui::BeginTable("##table", 7, flags, ImVec2(-1, 0)))
		{
			ImGui::TableSetupColumn("Channel", ImGuiTableColumnFlags_WidthFixed, 75.0f);
			ImGui::TableSetupColumn("Label", ImGuiTableColumnFlags_WidthFixed, 75.0f);			
			ImGui::TableSetupColumn("SDI", ImGuiTableColumnFlags_WidthFixed, 50.0f);
			ImGui::TableSetupColumn("Data", ImGuiTableColumnFlags_WidthFixed, 100.0f);
			ImGui::TableSetupColumn("SSM", ImGuiTableColumnFlags_WidthFixed, 50.0f);
			ImGui::TableSetupColumn("Parity", ImGuiTableColumnFlags_WidthFixed, 50.0f);
			ImGui::TableSetupColumn("Period");
			ImGui::TableHeadersRow();
			ImPlot::PushColormap(ImPlotColormap_Viridis);

			for (auto& [channel, flow] : labels)
			{
				for (auto& [label, dt] : flow)
				{
					ImGui::TableNextRow();

					// Создаем уникальный ID для строки
					ImGui::PushID(static_cast<int>(channel * 1000 + label));

					ImGui::TableSetColumnIndex(0);
					oss << "RX" << channel + 1;
					ImGui::Text("%s", oss.str().c_str());
					oss.str(""); oss.clear();

					ImGui::TableSetColumnIndex(1);
					oss << std::oct << label;
					ImGui::Text("%s", oss.str().c_str());
					oss.str(""); oss.clear();

					ImGui::TableSetColumnIndex(2);
					oss << "0x" << std::hex << std::uppercase << dt.word.bits.sdi;
					ImGui::Text("%s", oss.str().c_str());
					oss.str(""); oss.clear();

					ImGui::TableSetColumnIndex(3);
					oss << "0x" << std::hex << std::uppercase << dt.word.bits.data;
					ImGui::Text("%s", oss.str().c_str());
					oss.str(""); oss.clear();

					ImGui::TableSetColumnIndex(4);
					oss << "0x" << std::hex << std::uppercase << dt.word.bits.ssm;
					ImGui::Text("%s", oss.str().c_str());
					oss.str(""); oss.clear();

					ImGui::TableSetColumnIndex(5);
					oss << dt.word.bits.parity;
					ImGui::Text("%s", oss.str().c_str());
					oss.str(""); oss.clear();

					ImGui::TableSetColumnIndex(6);
					oss << std::dec << dt.delta << " ms";
					ImGui::Text("%s", oss.str().c_str());
					oss.str(""); oss.clear();
			
					ImGui::TableSetColumnIndex(0);
					ImGui::Selectable("##row_selectable", false, ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowOverlap);

					// Открываем попап при правом клике на строку
					if (ImGui::BeginPopupContextItem("##row_context"))
					{
						constexpr ImGuiTableFlags flags = ImGuiTableFlags_BordersOuter | ImGuiTableFlags_BordersV |
							ImGuiTableFlags_RowBg | ImGuiTableFlags_Resizable;

						// ТАБЛИЦА БИТОВ С ОДИНАКОВЫМ РАЗМЕРОМ ЯЧЕЕК
						if (ImGui::BeginTable("##bit_table", 32, flags, ImVec2(-1, 0)))
						{
							// Устанавливаем одинаковую ширину для всех колонок
							for (int i = 0; i < 32; i++) 
							{
								ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthFixed, 20.0f); // Фиксированная ширина
							}

							// Заголовок - номера битов (одинаковый размер ячеек)
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
								bool bit_value = (dt.word.value >> i) & 1;

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
			
			ImPlot::PopColormap();
			ImGui::EndTable();
		}
		
		ImGui::PopStyleVar(2);
	}
	ImGui::PopStyleColor();

	ImGui::End();
}

void UI::SetPanelSizeAndPosition(int corner, float width, float height, float x_offset, float y_offset)
{
	ImGuiIO& io = ImGui::GetIO();

	float MenuHeight = 0.0f;
	ImVec2 DispSize = io.DisplaySize;

	float PanelW = round(DispSize.x * width);
	float PanelH = DispSize.y * height;

	ImVec2 PanelSize = ImVec2(
		PanelW,
		PanelH
	);

	ImVec2 PanelPos = ImVec2(
		(corner & 1) ? DispSize.x + round(DispSize.x * x_offset) : round(DispSize.x * x_offset),
		(corner & 2) ? DispSize.y + MenuHeight + DispSize.y * y_offset : MenuHeight + DispSize.y * y_offset
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

	labels.clear();
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

	std::string str{ buf->begin(), buf->end() };
	int pos;

	std::stringstream ss;

	std::chrono::steady_clock::time_point stamp = std::chrono::steady_clock::now();
	
	while (str.size() > 0 && ThreadsAllowed)
	{
		pos = str.find('|');

		if (pos != -1)
		{
			ss << std::string(str.begin(), str.begin() + pos - 1);
			ss >> word;

			channel = std::stoi(std::string(str.begin() + pos - 1, str.begin() + pos));

			str = std::string(str.begin() + pos + 1, str.end());
				
			a429.word.value = word;

			mtx.lock();
			labels[channel][a429.word.bits.label].word = a429.word;
			labels[channel][a429.word.bits.label].delta_buf.push_back(std::chrono::duration_cast<std::chrono::milliseconds>(stamp - labels[channel][a429.word.bits.label].stamp).count());
			labels[channel][a429.word.bits.label].stamp = stamp;

			if (labels[channel][a429.word.bits.label].delta_buf.size() > 2)
			{
				labels[channel][a429.word.bits.label].delta = avg(labels[channel][a429.word.bits.label].delta_buf);
				labels[channel][a429.word.bits.label].delta_buf.erase(labels[channel][a429.word.bits.label].delta_buf.begin());			
			}
			mtx.unlock();

			ss.str("");
			ss.clear();
		}
	}
}
