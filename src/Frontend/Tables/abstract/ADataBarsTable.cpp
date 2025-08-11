#include <Frontend/Tables/abstract/ADataBarsTable.h>
#include <Systems/SettingsDialog.h>
#include <Models/DataBase.h>

ADataBarsTable::ADataBarsTable(const char* vTableName, const int32_t& vColummCount) : ADataTable(vTableName,vColummCount)  {

}

double ADataBarsTable::m_computeMaxPrice() {
    double max_price = 0.0;
    for (int32_t idx = m_GetListClipper().DisplayStart; idx < m_GetListClipper().DisplayEnd; ++idx) {
        if (idx < 0) {
            continue;
        }
        const auto& as = std::abs(m_getItemBarAmount(idx));
        if (idx == 0) {
            max_price = as;
        } else if (as > max_price) {
            max_price = as;
        }
    }
    return max_price;
}

void ADataBarsTable::m_drawColumnBars(const double vAmount, const double vMaxAmount, const float vColumNWidth) {
    ImGui::TableNextColumn();
    auto drawListPtr = ImGui::GetWindowDrawList();
    const auto& cursor = ImGui::GetCursorScreenPos();
    auto& g = *GImGui;
    auto* tbl_ptr = g.CurrentTable;
    const auto& table_column = tbl_ptr->Columns[tbl_ptr->CurrentColumn];
    auto column_width = vColumNWidth;
    if (column_width < 0.0f) {
        column_width = table_column.MaxX - table_column.MinX;
    }
    const ImVec2 pMin(cursor.x, cursor.y + m_GetTextHeight() * 0.1f);
    const ImVec2 pMax(cursor.x + column_width, cursor.y + m_GetTextHeight() * 0.9f);
    const float pMidX((pMin.x + pMax.x) * 0.5f);
    ImGui::SetCursorScreenPos(pMin);
    const float bw(column_width * 0.5f * (float)std::abs(vAmount) / vMaxAmount);
    if (vAmount < 0.0) {
        drawListPtr->AddRectFilled(ImVec2(pMidX - bw, pMin.y), ImVec2(pMidX, pMax.y), ImGui::GetColorU32(ImGui::CustomStyle::BadColor));
    } else if (vAmount > 0.0) {
        drawListPtr->AddRectFilled(ImVec2(pMidX, pMin.y), ImVec2(pMidX + bw, pMax.y), ImGui::GetColorU32(ImGui::CustomStyle::GoodColor));
    }
    ImGui::SetCursorScreenPos(pMax);
    ImGui::Dummy(ImVec2(0, 0)); // https://github.com/ocornut/imgui/issues/5548
}
