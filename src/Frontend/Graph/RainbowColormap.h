#pragma once

#include <map>
#include <string>
#include <vector>
#include <cstdint>
#include <imguipack.h>
#include <ezlibs/ezTools.hpp>

// shared helper : returns an ImPlot colormap built from ez::getRainBowColor(idx, vSize).
// ImPlot has no RemoveColormap, so we cache by size : each distinct size triggers a single
// registration ; subsequent refreshes at the same size reuse the cached colormap. colors for
// a given size are deterministic (getRainBowColor is pure), so this reuse is safe.
inline ImPlotColormap getOrCreateEzRainbowColormap(int32_t vSize) {
    static std::map<int32_t, ImPlotColormap> sCache;
    if (vSize < 1) {
        vSize = 1;
    }
    const auto it = sCache.find(vSize);
    if (it != sCache.end()) {
        return it->second;
    }
    std::vector<ImVec4> colors(static_cast<size_t>(vSize));
    for (int32_t idx = 0; idx < vSize; ++idx) {
        const ez::fvec4 c = ez::getRainBowColor(idx, vSize);
        colors[static_cast<size_t>(idx)] = ImVec4(c.x, c.y, c.z, c.w);
    }
    // ImPlot::AddColormap asserts on size > 1 -> pad a single-entry palette by duplicating the color
    if (colors.size() < 2u) {
        colors.push_back(colors.front());
    }
    const std::string name = "ezRainbow_" + std::to_string(vSize);
    const ImPlotColormap cmap = ImPlot::AddColormap(name.c_str(), colors.data(), static_cast<int>(colors.size()), true);
    sCache[vSize] = cmap;
    return cmap;
}
