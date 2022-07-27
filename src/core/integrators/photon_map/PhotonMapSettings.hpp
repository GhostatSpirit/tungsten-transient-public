#ifndef PHOTONMAPSETTINGS_HPP_
#define PHOTONMAPSETTINGS_HPP_

#include "Photon.hpp"

#include "integrators/TraceSettings.hpp"

#include "io/JsonObject.hpp"

#include "StringableEnum.hpp"

#include <tinyformat/tinyformat.hpp>

namespace Tungsten {

struct PhotonMapSettings : public TraceSettings
{
    typedef StringableEnum<VolumePhotonEnum> VolumePhotonType;
    friend VolumePhotonType;

    uint32 photonCount;
    uint32 volumePhotonCount;
    uint32 gatherCount;
    float gatherRadius;
    float volumeGatherRadius;
    VolumePhotonType volumePhotonType;
    bool includeSurfaces;
    bool lowOrderScattering;
    bool fixedVolumeRadius;
    bool useGrid;
    bool useFrustumGrid;
    int gridMemBudgetKb;
    bool deltaTimeGate;
    float transientTimeCenter;
    float transientTimeWidth;
    float transientTimeBeg;
    float transientTimeEnd;
    float invTransientTimeWidth;
    float vaccumSpeedOfLight;
    bool excludeNonMIS;         // exclude light transport we cannot MIS with

    PhotonMapSettings()
    : photonCount(1000000),
      volumePhotonCount(1000000),
      gatherCount(20),
      gatherRadius(1e30f),
      volumeGatherRadius(gatherRadius),
      volumePhotonType("points"),
      includeSurfaces(true),
      lowOrderScattering(true),
      fixedVolumeRadius(false),
      useGrid(false),
      useFrustumGrid(false),
      gridMemBudgetKb(32*1024),
      deltaTimeGate(false),
      transientTimeCenter(0.0f),
      transientTimeWidth(0.1f),
      transientTimeBeg(0.0f),
      transientTimeEnd(0.0f),
      invTransientTimeWidth(1.0f/ transientTimeWidth),
      vaccumSpeedOfLight(1.f),
      excludeNonMIS(false)
    {
    }

    void setTimeCenter(float timeCenter)
    {
        transientTimeCenter = timeCenter;
        transientTimeBeg = deltaTimeGate ? transientTimeCenter : transientTimeCenter - 0.5f * transientTimeWidth;
        transientTimeEnd = deltaTimeGate ? transientTimeCenter : transientTimeCenter + 0.5f * transientTimeWidth;
    }

    void fromJson(JsonPtr value)
    {
        TraceSettings::fromJson(value);
        value.getField("photon_count", photonCount);
        value.getField("volume_photon_count", volumePhotonCount);
        value.getField("gather_photon_count", gatherCount);
        if (auto type = value["volume_photon_type"])
            volumePhotonType = type;
        bool gatherRadiusSet = value.getField("gather_radius", gatherRadius);
        if (!value.getField("volume_gather_radius", volumeGatherRadius) && gatherRadiusSet)
            volumeGatherRadius = gatherRadius;
        value.getField("low_order_scattering", lowOrderScattering);
        value.getField("include_surfaces", includeSurfaces);
        value.getField("fixed_volume_radius", fixedVolumeRadius);
        value.getField("use_grid", useGrid);
        value.getField("use_frustum_grid", useFrustumGrid);
        value.getField("grid_memory", gridMemBudgetKb);

        if (useFrustumGrid && volumePhotonType == VOLUME_POINTS)
            value.parseError("Photon points cannot be used with a frustum aligned grid");

        value.getField("transient_time_center", transientTimeCenter);

        value.getField("transient_time_width", transientTimeWidth);
        value.getField("delta_time_gate", deltaTimeGate);
        invTransientTimeWidth = deltaTimeGate ? 1.f : 1.0f / transientTimeWidth;
        setTimeCenter(transientTimeCenter);

        value.getField("vaccum_speed_of_light", vaccumSpeedOfLight);
        value.getField("exclude_non_mis", excludeNonMIS);

        static const VolumePhotonType deltaTimeGateSupportedTypes [] = {
            VOLUME_BEAMS,
            VOLUME_VOLUMES,
            VOLUME_HYPERVOLUMES,
            VOLUME_BALLS,
            VOLUME_VOLUMES_BALLS,
            VOLUME_MIS_VOLUMES_BALLS
        };

        if (deltaTimeGate)
        {
            auto iter = std::find(std::begin(deltaTimeGateSupportedTypes), std::end(deltaTimeGateSupportedTypes), volumePhotonType);
            if (iter == std::end(deltaTimeGateSupportedTypes))
            {
                std::string supportedTypesString {};
                for (auto iterSupported = std::begin(deltaTimeGateSupportedTypes); iterSupported != std::end(deltaTimeGateSupportedTypes); ++iterSupported)
                {
                    supportedTypesString += (*iterSupported).toString();
                    supportedTypesString += " ";
                }
                std::string errorString = tfm::format("Delta time gate mode is not supported by volume photon type \"%s\". ", volumePhotonType.toString());
                errorString += tfm::format("Supported types are: %s", supportedTypesString);
                value.parseError(errorString);
            }
        }
    }

    rapidjson::Value toJson(rapidjson::Document::AllocatorType &allocator) const
    {
        return JsonObject{TraceSettings::toJson(allocator), allocator,
            "type", "photon_map",
            "photon_count", photonCount,
            "volume_photon_count", volumePhotonCount,
            "gather_photon_count", gatherCount,
            "gather_radius", gatherRadius,
            "volume_gather_radius", volumeGatherRadius,
            "volume_photon_type", volumePhotonType.toString(),
            "low_order_scattering", lowOrderScattering,
            "include_surfaces", includeSurfaces,
            "fixed_volume_radius", fixedVolumeRadius,
            "use_grid", useGrid,
            "use_frustum_grid", useFrustumGrid,
            "grid_memory", gridMemBudgetKb,
            "transient_time_center", transientTimeCenter,
            "transient_time_width", transientTimeWidth,
            "vaccum_speed_of_light", vaccumSpeedOfLight,
        };
    }
};

}

#endif /* PHOTONMAPSETTINGS_HPP_ */
