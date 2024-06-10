// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Budget Analyzer for C, C++ and C#: http://www.viva64.com

#include <Panes/DebitCreditPane.h>

#include <cinttypes>  // printf zu

#include <ctools/Logger.h>

#include <Models/DataBase.h>

#include <Project/ProjectFile.h>
#include <Systems/SettingsDialog.h>

DebitCreditPane::DebitCreditPane() = default;
DebitCreditPane::~DebitCreditPane() {
    Unit();
}

bool DebitCreditPane::Init() {
    m_DateFormatCombo.init(  //
        1,                   // MONTHS
        {
            "DAYS",    //
            "MONTHS",  //
            "YEARS",   //
        });
    assert(m_DateFormatCombo.size() == (size_t)DateFormat::Count);
    m_GroupByCombo.init(  //
        0,
        {
            "DATES",        //
            "ENTITIES",   //
            "OPERATIONS",   //
            "CATEGORIES",   //
            "DESCRIPTIONS"  //
        });
    assert(m_GroupByCombo.size() == (size_t)GroupBy::Count);
    return true;
}

void DebitCreditPane::Unit() {
    m_BarDatas.clear();
    m_Transactions.clear();
}

///////////////////////////////////////////////////////////////////////////////////
//// IMGUI PANE ///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////

bool DebitCreditPane::DrawPanes(const uint32_t& /*vCurrentFrame*/, bool* vOpened, ImGuiContext* vContextPtr, void* /*vUserDatas*/) {
    ImGui::SetCurrentContext(vContextPtr);
    bool change = false;
    if (vOpened != nullptr && *vOpened) {
        static ImGuiWindowFlags flags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_MenuBar;
        if (ImGui::Begin(GetName().c_str(), vOpened, flags)) {
#ifdef USE_DECORATIONS_FOR_RESIZE_CHILD_WINDOWS
            auto win = ImGui::GetCurrentWindowRead();
            if (win->Viewport->Idx != 0)
                flags |= ImGuiWindowFlags_NoResize;  // | ImGuiWindowFlags_NoTitleBar;
            else
                flags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_MenuBar;
#endif
        }

        if (ProjectFile::Instance()->IsProjectLoaded()) {
            m_drawMenu();
            m_drawBuySellGraph();
            m_drawBuySellList();
        }

        ImGui::End();
    }
    return change;
}

bool DebitCreditPane::DrawOverlays(const uint32_t& /*vCurrentFrame*/, const ImRect& /*vRect*/, ImGuiContext* vContextPtr, void* /*vUserDatas*/) {
    ImGui::SetCurrentContext(vContextPtr);
    return false;
}

bool DebitCreditPane::DrawDialogsAndPopups(const uint32_t& /*vCurrentFrame*/, const ImRect& /*vRect*/, ImGuiContext* vContextPtr, void* /*vUserDatas*/) {
    ImGui::SetCurrentContext(vContextPtr);    
    return false;
}

bool DebitCreditPane::DrawWidgets(const uint32_t& /*vCurrentFrame*/, ImGuiContext* vContextPtr, void* /*vUserDatas*/) {
    ImGui::SetCurrentContext(vContextPtr);
    return false;
}

void DebitCreditPane::Load() {
    m_refreshDatas();
}

void DebitCreditPane::m_refreshDatas() {
    m_UpdateAccounts();
}

void DebitCreditPane::m_drawMenu() {
    if (ImGui::BeginMenuBar()) {
        if (m_AccountsCombo.displayCombo(200.0f, "Accounts")) {
            m_UpdateAccounts();
        }
        if (m_DateFormatCombo.displayCombo(200.0f, " Date format")) {
            m_UpdateTransactions(m_Accounts.at(m_AccountsCombo.getIndex()).id);
        }
        if (m_GroupByCombo.displayCombo(200.0f, " Group by")) {
            m_UpdateTransactions(m_Accounts.at(m_AccountsCombo.getIndex()).id);
        }
        if (ImGui::SliderDoubleDefaultCompact(150.0f, "Bar Width", &m_BarHalfWidthPercent, 0.0, 0.5, 0.3)) {
            m_ComptueBarsWidth();
        }
        ImGui::EndMenuBar();
    }
}

void DebitCreditPane::m_drawBuySellGraph() {
    static auto flags = ImPlotBarGroupsFlags_Horizontal | ImPlotBarGroupsFlags_Stacked;
    static const char* labels[] = {"Debit", "Credit"};
    const auto height = ImGui::GetContentRegionAvail().y * 0.3f;
    if (ImPlot::BeginPlot("##Histograms", ImVec2(-1.0f, height))) {
        // get ImGui window DrawList
        ImDrawList* draw_list = ImPlot::GetPlotDrawList();

        // custom tool
        /*if (ImPlot::IsPlotHovered() && tooltip) {
            ImPlotPoint mouse = ImPlot::GetPlotMousePos();
            mouse.x = ImPlot::RoundTime(ImPlotTime::FromDouble(mouse.x), ImPlotTimeUnit_Day).ToDouble();
            float tool_l = ImPlot::PlotToPixels(mouse.x - half_width * 1.5, mouse.y).x;
            float tool_r = ImPlot::PlotToPixels(mouse.x + half_width * 1.5, mouse.y).x;
            float tool_t = ImPlot::GetPlotPos().y;
            float tool_b = tool_t + ImPlot::GetPlotSize().y;
            ImPlot::PushPlotClipRect();
            draw_list->AddRectFilled(ImVec2(tool_l, tool_t), ImVec2(tool_r, tool_b), IM_COL32(128, 128, 128, 64));
            ImPlot::PopPlotClipRect();
            // find mouse location index
            int idx = BinarySearch(xs, 0, count - 1, mouse.x);
            // render tool tip (won't be affected by plot clip rect)
            if (idx != -1) {
                ImGui::BeginTooltip();
                char buff[32];
                ImPlot::FormatDate(ImPlotTime::FromDouble(xs[idx]), buff, 32, ImPlotDateFmt_DayMoYr, ImPlot::GetStyle().UseISO8601);
                ImGui::Text("Day:   %s", buff);
                ImGui::Text("Open:  $%.2f", opens[idx]);
                ImGui::Text("Close: $%.2f", closes[idx]);
                ImGui::Text("Low:   $%.2f", lows[idx]);
                ImGui::Text("High:  $%.2f", highs[idx]);
                ImGui::EndTooltip();
            }
        }*/

        const char* x_axis_label = nullptr;
        const auto group_by = (GroupBy)m_GroupByCombo.getIndex(); 
        if (group_by == GroupBy::DATES) {
            ImPlot::SetupAxisScale(ImAxis_X1, ImPlotScale_Time);
            x_axis_label = "Dates";
        }
        ImPlot::SetupAxes(x_axis_label, "Amount", ImPlotAxisFlags_AutoFit, ImPlotAxisFlags_AutoFit);

        if (ImPlot::BeginItem("Debit")) {
            ImPlot::GetCurrentItem()->Color = IM_COL32(128, 64, 64, 255);
            for (int i = 0; i < m_BarDatas.count; ++i) {
                const auto date = m_BarDatas.dates[i];
                if (ImPlot::FitThisFrame()) {
                    ImPlot::FitPoint(ImPlotPoint(m_BarDatas.dates[i], 0.0));
                    ImPlot::FitPoint(ImPlotPoint(m_BarDatas.dates[i], m_BarDatas.debits[i]));
                }
                ImVec2 zero_pos = ImPlot::PlotToPixels(date - m_BarDatas.half_width, 0.0);
                ImVec2 debit_pos = ImPlot::PlotToPixels(date + m_BarDatas.half_width, m_BarDatas.debits[i]);
                draw_list->AddRectFilled(zero_pos, debit_pos, ImPlot::GetCurrentItem()->Color);
            }
            ImPlot::EndItem();
        }

        if (ImPlot::BeginItem("Credit")) {
            ImPlot::GetCurrentItem()->Color = IM_COL32(64, 128, 64, 255);
            for (int i = 0; i < m_BarDatas.count; ++i) {
                const auto date = m_BarDatas.dates[i];
                if (ImPlot::FitThisFrame()) {
                    ImPlot::FitPoint(ImPlotPoint(m_BarDatas.dates[i], 0.0));
                    ImPlot::FitPoint(ImPlotPoint(m_BarDatas.dates[i], m_BarDatas.credits[i]));
                }
                ImVec2 zero_pos = ImPlot::PlotToPixels(date - m_BarDatas.half_width, 0.0);
                ImVec2 credit_pos = ImPlot::PlotToPixels(date + m_BarDatas.half_width, m_BarDatas.credits[i]);
                draw_list->AddRectFilled(zero_pos, credit_pos, ImPlot::GetCurrentItem()->Color);
            }
            ImPlot::EndItem();
        }

        ImPlot::SetNextLineStyle(ImVec4(1.0f, 1.0f, 1.0f, 1.0f), 2.0f);
        ImPlot::PlotLine("Amount", m_BarDatas.dates.data(), m_BarDatas.amounts.data(), m_BarDatas.count);
        ImPlot::EndPlot();
    }
}

void DebitCreditPane::m_drawBuySellList() {
    ImGui::PushStyleVar(ImGuiStyleVar_GrabMinSize, 30.0f);
    static auto flags = ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY;
    if (ImGui::BeginTable("##Transactions", 8, flags)) {
        ImGui::TableSetupScrollFreeze(0, 1);
        ImGui::TableSetupColumn("Dates", ImGuiTableColumnFlags_WidthFixed);
        ImGui::TableSetupColumn("Descriptions", ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableSetupColumn("Entity", ImGuiTableColumnFlags_WidthFixed);
        ImGui::TableSetupColumn("Category", ImGuiTableColumnFlags_WidthFixed);
        ImGui::TableSetupColumn("Operation", ImGuiTableColumnFlags_WidthFixed);
        ImGui::TableSetupColumn("Debit", ImGuiTableColumnFlags_WidthFixed);
        ImGui::TableSetupColumn("Credit", ImGuiTableColumnFlags_WidthFixed);
        ImGui::TableSetupColumn("Amount", ImGuiTableColumnFlags_WidthFixed);
        ImGui::TableHeadersRow();
        int32_t idx = 0;
        ImU32 color;
        bool colored = false;
        const float& bar_column_width = 100.0f;
        auto drawListPtr = ImGui::GetWindowDrawList();
        const float& text_h = ImGui::GetTextLineHeight();
        const float& item_h = ImGui::GetTextLineHeightWithSpacing();
        const auto& bad_color = ImGui::GetColorU32(ImVec4(1, 0, 0, 1));
        const auto& good_color = ImGui::GetColorU32(ImVec4(0, 1, 0, 1));
        m_TransactionsListClipper.Begin((int)m_Transactions.size(), item_h);
        while (m_TransactionsListClipper.Step()) {
            for (idx = m_TransactionsListClipper.DisplayStart; idx < m_TransactionsListClipper.DisplayEnd; ++idx) {
                if (idx < 0) {
                    continue;
                }

                auto& t = m_Transactions.at(idx);

                ImGui::TableNextRow();

                ImGui::TableNextColumn();
                { ImGui::Text(t.date.c_str()); }

                ImGui::TableNextColumn();
                {
                    ImGui::PushID(&t);
                    auto is_selected = false;
                    ImGui::Selectable(t.description.c_str(), &is_selected, ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowOverlap);
                    ImGui::HideByFilledRectForHiddenMode(SettingsDialog::Instance()->isHiddenMode(), "%s", t.description.c_str());
                    ImGui::PopID();
                }

                ImGui::TableNextColumn();
                {
                    ImGui::Text(t.entity.c_str());
                    ImGui::HideByFilledRectForHiddenMode(SettingsDialog::Instance()->isHiddenMode(), "%s", t.entity.c_str());
                }

                ImGui::TableNextColumn();
                {
                    ImGui::Text(t.category.c_str());
                    ImGui::HideByFilledRectForHiddenMode(SettingsDialog::Instance()->isHiddenMode(), "%s", t.category.c_str());
                }

                ImGui::TableNextColumn();
                {
                    ImGui::Text(t.operation.c_str());
                    ImGui::HideByFilledRectForHiddenMode(SettingsDialog::Instance()->isHiddenMode(), "%s", t.operation.c_str());
                }

                ImGui::TableNextColumn();
                {
                    if (t.debit < 0.0) {
                        ImGui::PushStyleColor(ImGuiCol_Text, bad_color);
                        ImGui::Text("%.2f", t.debit);
                        ImGui::HideByFilledRectForHiddenMode(SettingsDialog::Instance()->isHiddenMode(), "%.2f", t.debit);
                        ImGui::PopStyleColor();
                    }
                }

                ImGui::TableNextColumn();
                {
                    if (t.credit > 0.0) {
                        ImGui::PushStyleColor(ImGuiCol_Text, good_color);
                        ImGui::Text("%.2f", t.credit);
                        ImGui::HideByFilledRectForHiddenMode(SettingsDialog::Instance()->isHiddenMode(), "%.2f", t.credit);
                        ImGui::PopStyleColor();
                    }
                }

                ImGui::TableNextColumn();
                {
                    colored = false;
                    if (t.amount < 0.0) {
                        ImGui::PushStyleColor(ImGuiCol_Text, bad_color);
                        colored = true;
                    } else if (t.amount > 0.0) {
                        ImGui::PushStyleColor(ImGuiCol_Text, good_color);
                        colored = true;
                    }
                    ImGui::Text("%.2f", t.amount);
                    ImGui::HideByFilledRectForHiddenMode(SettingsDialog::Instance()->isHiddenMode(), "%.2f", t.amount);
                    if (colored) {
                        ImGui::PopStyleColor();
                    }
                }
            }
        }
        m_TransactionsListClipper.End();
        ImGui::EndTable();
    }
    ImGui::PopStyleVar();
}

void DebitCreditPane::m_UpdateAccounts() {
    m_Accounts.clear();
    m_AccountsCombo.clear();
    DataBase::Instance()->GetAccounts(  //
        [this](const RowID& vRowID,
               const BankName& vBankName,
               const BankAgency& vBankAgency,
               const AccountType& vAccountType,
               const AccountName& vAccountName,
               const AccountNumber& vAccountNumber,
               const AccounBaseSolde& vBaseSolde,
               const TransactionsCount& vCount) {  //
            Account a;
            a.id = vRowID;
            a.bank = vBankName;
            a.agency = vBankAgency;
            a.type = vAccountType;
            a.name = vAccountName;
            a.number = vAccountNumber;
            a.base_solde = vBaseSolde;
            a.count = vCount;
            m_Accounts.push_back(a);
            m_AccountsCombo.getArrayRef().push_back(vAccountNumber);
        });
    m_UpdateTransactions(m_Accounts.at(m_AccountsCombo.getIndex()).id);
}

void DebitCreditPane::m_UpdateTransactions(const RowID& vAccountID) {
    m_Transactions.clear();
    DataBase::Instance()->GetGroupedTransactions(  //
        vAccountID,
        (GroupBy)m_GroupByCombo.getIndex(),        //
        (DateFormat)m_DateFormatCombo.getIndex(),  //
        [this](                                    //
            const RowID& vRowID,
            const TransactionDate& vTransactionDate,
            const TransactionDescription& vTransactionDescription,
            const EntityName& vEntityName,
            const CategoryName& vCategoryName,
            const OperationName& vOperationName,
            const TransactionDebit& vTransactionDebit,
            const TransactionCredit& vTransactionCredit) {
            GroupedTransaction t;
            t.id = vRowID;
            t.date = vTransactionDate;
            t.description = vTransactionDescription;
            t.entity = vEntityName;
            t.category = vCategoryName;
            t.operation = vOperationName;
            t.debit = vTransactionDebit;
            t.credit = vTransactionCredit;
            t.amount = vTransactionDebit + vTransactionCredit;
            m_Transactions.push_back(t);
        });
    m_UpdateBarDatas();
    LogVarDebugInfo("Count Transactions : %u", (uint32_t)m_Transactions.size());
}

void DebitCreditPane::m_UpdateBarDatas() {
    if (!m_Transactions.empty()) {
        m_BarDatas.clear();
        const auto group_by = (GroupBy)m_GroupByCombo.getIndex();        
        const auto& date_format = DataBase::Instance()->GetFormatDate(  //
            (DateFormat)m_DateFormatCombo.getIndex());
        m_BarDatas.labels.reserve(m_Transactions.size());
        m_BarDatas.debits.reserve(m_Transactions.size());
        m_BarDatas.credits.reserve(m_Transactions.size());
        m_BarDatas.amounts.reserve(m_Transactions.size());
        double idx = 0.0;
        for (const auto& t : m_Transactions) {
            m_BarDatas.labels.push_back(t.date.c_str());
            m_BarDatas.debits.push_back(t.debit);
            m_BarDatas.credits.push_back(t.credit);
            m_BarDatas.amounts.push_back(t.debit + t.credit);
            if (group_by == GroupBy::DATES) {
                time_t time;
                if (ct::iso8601ToEpoch(t.date, date_format, time)) {
                    m_BarDatas.dates.push_back(ImPlotTime(time).ToDouble());
                } else {
                    CTOOL_DEBUG_BREAK;
                }
            } else {
                m_BarDatas.dates.push_back(idx);
            }
            ++idx;
        }
        m_BarDatas.count = static_cast<int32_t>(m_BarDatas.dates.size());
        m_BarDatas.values.reserve(m_BarDatas.count * 2U);
        for (const auto& d : m_BarDatas.debits) {
            m_BarDatas.values.push_back(d);
        }
        for (const auto& c : m_BarDatas.credits) {
            m_BarDatas.values.push_back(c);
        }
        m_ComptueBarsWidth();
    }
}

void DebitCreditPane::m_ComptueBarsWidth() {
    if (m_BarDatas.count > 1) {
        for (int i = 1; i < m_BarDatas.count; ++i) {
            m_BarDatas.half_width += (m_BarDatas.dates[i - 1] - m_BarDatas.dates[i]) * m_BarHalfWidthPercent;
        }
        m_BarDatas.half_width /= (double)m_BarDatas.count;
    }
}
