#ifndef PHOTONTRACER_HPP_
#define PHOTONTRACER_HPP_

#include "PhotonMapSettings.hpp"
#include "FrustumBinner.hpp"
#include "PhotonRange.hpp"
#include "KdTree.hpp"
#include "Photon.hpp"

#include "integrators/TraceBase.hpp"

#include <unordered_map>

namespace Tungsten {

namespace Bvh {
class BinaryBvh;
}
class GridAccel;

class HashedShadowCache
{
    std::unordered_map<uint64, float> _cache;

public:
    HashedShadowCache(uint64 initialSize)
    {
        _cache.reserve(initialSize);
    }

    void clear()
    {
        _cache.clear();
    }

    template<typename Tracer>
    inline float hitDistance(uint32 photon, uint32 bin, Tracer tracer)
    {
        uint64 key = (uint64(photon) << 32ull) | uint64(bin);
        auto iter = _cache.find(key);
        if (iter == _cache.end()) {
            float dist = tracer();
            _cache.insert(std::make_pair(key, dist));
            return dist;
        } else {
            return iter->second;
        }
    }
};

class LinearShadowCache
{
    const uint32 MaxCacheBins = 1024*1024;

    std::unique_ptr<uint32[]> _photonIndices;
    std::unique_ptr<float[]> _distances;

public:
    LinearShadowCache()
    : _photonIndices(new uint32[MaxCacheBins]),
      _distances(new float[MaxCacheBins])
    {
        clear();
    }

    void clear()
    {
        std::memset(_photonIndices.get(), 0, MaxCacheBins*sizeof(_photonIndices[0]));
    }

    template<typename Tracer>
    inline float hitDistance(uint32 photon, uint32 bin, Tracer tracer)
    {
        if (bin < MaxCacheBins && _photonIndices[bin] == photon) {
            return _distances[bin];
        } else {
            float dist = tracer();
            _photonIndices[bin] = photon;
            _distances[bin] = dist;
            return dist;
        }
    }
};

template <bool isTransient>
class PhotonTracer : public TraceBase
{
    static CONSTEXPR bool isSteadyState = !isTransient;

    typedef typename std::conditional<isSteadyState, SteadyVolumePhotonRange, Transient::TransientVolumePhotonRange>::type VolumePhotonRange;
    typedef typename std::conditional<isSteadyState, SteadyPathPhotonRange, Transient::TransientPathPhotonRange>::type PathPhotonRange;
    typedef typename std::conditional<isSteadyState, SteadyVolumePhoton, Transient::TransientVolumePhoton>::type VolumePhoton;
    typedef typename std::conditional<isSteadyState, SteadyPathPhoton, Transient::TransientPathPhoton>::type PathPhoton;
    typedef typename std::conditional<isSteadyState, SteadyPhotonBeam, Transient::TransientPhotonBeam>::type PhotonBeam;
    typedef typename std::conditional<isSteadyState, SteadyPhotonPlane0D, Transient::TransientPhotonPlane0D>::type PhotonPlane0D;
    typedef typename std::conditional<isSteadyState, SteadyPhotonVolume, Transient::TransientPhotonVolume>::type PhotonVolume;
    typedef typename std::conditional<isSteadyState, SteadyPhotonHyperVolume, Transient::TransientPhotonHyperVolume>::type PhotonHyperVolume;
    typedef typename std::conditional<isSteadyState, SteadyPhotonBall, Transient::TransientPhotonBall>::type PhotonBall;

    PhotonMapSettings _settings;
    uint32 _mailIdx;
    std::unique_ptr<const Photon *[]> _photonQuery;
    std::unique_ptr<float[]> _distanceQuery;
    std::unique_ptr<uint32[]> _mailboxes;

    LinearShadowCache _directCache;
    HashedShadowCache _indirectCache;

    FrustumBinner _frustumGrid;

    void clearCache();

public:
    PhotonTracer(TraceableScene *scene, const PhotonMapSettings &settings, uint32 threadId);

    void evalPrimaryRays(const PhotonBeam *beams, const PhotonPlane0D *planes0D, const PhotonPlane1D *planes1D,
            uint32 start, uint32 end, float radius, const Ray *depthBuffer, PathSampleGenerator &sampler, float scale);

    Vec3f traceSensorPath(Vec2u pixel, const KdTree<Photon> &surfaceTree,
            const KdTree<VolumePhoton> *mediumTree, const Bvh::BinaryBvh *mediumBvh, const GridAccel *mediumGrid,
            const PhotonBeam *beams, const PhotonPlane0D *planes0D, const PhotonPlane1D *planes1D,
            const PhotonVolume *volumes, const PhotonHyperVolume *hyperVolumes, const PhotonBall *balls,
            PathSampleGenerator &sampler,
            float gatherRadius, float volumeGatherRadius, float beamTimeWidth,
            PhotonMapSettings::VolumePhotonType photonType, Ray &depthRay, bool useFrustumGrid);

    void tracePhotonPath(SurfacePhotonRange &surfaceRange, VolumePhotonRange &volumeRange,
            PathPhotonRange &pathRange, PathSampleGenerator &sampler);
};

}

#endif /* PHOTONTRACER_HPP_ */
