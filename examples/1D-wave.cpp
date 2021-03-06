#include <cmath>
#include <iostream>
#include <memory>
#include <vector>

#include <GaspiCxx/Runtime.hpp>
#include <GaspiCxx/group/Group.hpp>
#include <GaspiCxx/group/Rank.hpp>
#include <GaspiCxx/segment/Allocator.hpp>
#include <GaspiCxx/segment/Segment.hpp>
#include <GaspiCxx/singlesided/write/SourceBuffer.hpp>
#include <GaspiCxx/singlesided/write/TargetBuffer.hpp>
#include <GaspiCxx/utility/ScopedAllocation.hpp>

namespace parameter {

  static int const nInner1D (1000);
  static int const nBound1D (4);
  static int const nt       (1550);
  static float     dt       (1e-3);
  static float     dx       (5e+0);
  static float     c        (3.4e3);
}

template <typename GI>
GI div_roundup(GI numerator, GI denominator)
{ return ( numerator + denominator - static_cast<GI>(1) ) / denominator; }

template <class GI>
class UniformPartition
{

public:

  UniformPartition( GI begin, GI end, unsigned short nparts )
  : _begin(begin)
  , _nelems(end-begin)
  , _nparts(nparts)
  {}

  GI
  begin( unsigned short ipart) const
  { return _begin
        + div_roundup( static_cast<GI>(ipart) * _nelems, _nparts ); }

  GI
  end( unsigned short ipart ) const
  { return begin(ipart + 1); }

  GI
  nparts() const
  { return _nparts; }

  GI
  size(unsigned short ipart ) const
  { return end(ipart) - begin(ipart); }


private:

  GI _begin;
  GI _nelems;
  GI _nparts;

};

template < typename T >
class Field1D {

  public:

    Field1D
      ( int nInner1D
      , int nBound1D
      , gaspi::segment::Segment & segment
      , gaspi::CommunicationContext& comm_context
      , gaspi::group::Group const& group )
    : _comm_context(comm_context)
    , _group(group)
    , _nInner1D
        (UniformPartition<int>(0,nInner1D,_group.size())
           .size(_group.rank().get()))
    , _nBound1D
        (nBound1D)
    , _pField1D
        ( std::allocator_traits<decltype(segment.allocator())>
            ::rebind_alloc<T>(segment.allocator())
        , _nInner1D + 2 * _nBound1D )
    , _leftBoundary
        ( &_pField1D[_nBound1D]
        , segment
        , _nBound1D * sizeof(T) )
    , _rightBoundary
        ( &_pField1D[_nInner1D]
        , segment
        , _nBound1D * sizeof(T) )
    , _leftHalo
        ( &_pField1D[0]
        , segment
        , _nBound1D * sizeof(T) )
    , _rightHalo
        ( &_pField1D[_nInner1D+_nBound1D]
        , segment
        , _nBound1D * sizeof(T) )
    {
      gaspi::group::Rank
        rightNeighbour( ( _group.rank()
                        + _group.size()
                        + 1 ) % _group.size() );

      gaspi::group::Rank
        leftNeighbour ( ( _group.rank()
                        + _group.size()
                        - 1 ) % _group.size() );

      int leftHaloTag(1);
      int rightHaloTag(2);

      gaspi::singlesided::Endpoint::ConnectHandle leftHaloHandle
          ( _leftHalo.connectToRemoteSource
              ( _group
              , leftNeighbour
              , leftHaloTag ) );

      gaspi::singlesided::Endpoint::ConnectHandle leftBoundaryHandle
          ( _leftBoundary.connectToRemoteTarget
              ( _group
              , leftNeighbour
              , rightHaloTag ) );

      gaspi::singlesided::Endpoint::ConnectHandle rightBoundaryHandle
        ( _rightBoundary.connectToRemoteTarget
          ( _group
          , rightNeighbour
          , leftHaloTag ) );

      gaspi::singlesided::Endpoint::ConnectHandle rightHaloHandle
          ( _rightHalo.connectToRemoteSource
              ( _group
              , rightNeighbour
              , rightHaloTag ) );

      leftHaloHandle.waitForCompletion();
      leftBoundaryHandle.waitForCompletion();
      rightBoundaryHandle.waitForCompletion();
      rightHaloHandle.waitForCompletion();

      for( int i(0)
         ;     i<(_nInner1D+2*_nBound1D)
         ;   ++i ) {
        (*this)[i] = static_cast<T>(0);
      }
    }

    T &
    operator[]
      (int const & idx) {
      return _pField1D[idx];
    }

    T const &
    operator[]
      (int const & idx) const {
      return _pField1D[idx];
    }

    void
    updateInner
      ( T const kappahat
      , Field1D const & uOld ) {
      updateRange
        ( kappahat
        , uOld
        , 2 * _nBound1D
        , _nInner1D );
    }

    void
    updateLeftBoundary
      ( T const kappahat
      , Field1D const & uOld ) {
      updateRange
        ( kappahat
        , uOld
        , 1 * _nBound1D
        , 2 * _nBound1D );

      _leftBoundary.initTransfer(_comm_context);
    }

    void
    updateRightBoundary
      ( T const kappahat
      , Field1D const & uOld ) {
      updateRange
        ( kappahat
        , uOld
        , _nInner1D
        , _nInner1D + _nBound1D);

      _rightBoundary.initTransfer(_comm_context);
    }

    void
    waitForHalos
      () {

      _leftHalo .waitForCompletion();
      _rightHalo.waitForCompletion();
    }

  private:

    void
    updateRange
      ( T const kappahat
      , Field1D const & uOld
      , int const iFirst
      , int const iLast ) {
      for( int ix(iFirst)
         ;     ix<iLast
         ;   ++ix ) {
        (*this)[ix] = kappahat *
            ( -1./560.   * (uOld[ix-4]+uOld[ix+4])
              +8./315.   * (uOld[ix-3]+uOld[ix+3])
              -1./5.     * (uOld[ix-2]+uOld[ix+2])
              +8./5.     * (uOld[ix-1]+uOld[ix+1])
              -205./144. * (uOld[ix-0]+uOld[ix+0])
            ) + 2.*uOld[ix]-(*this)[ix];
      }
    }

    gaspi::CommunicationContext&            _comm_context;
    gaspi::group::Group        _group;
    int                        _nInner1D;
    int                        _nBound1D;

    gaspi::ScopedAllocation<T> _pField1D;

    gaspi::singlesided::write::SourceBuffer _leftBoundary;
    gaspi::singlesided::write::SourceBuffer _rightBoundary;

    gaspi::singlesided::write::TargetBuffer _leftHalo;
    gaspi::singlesided::write::TargetBuffer _rightHalo;

};

int
main
  ( int /*argc*/
  , char *[] /*argv*/) try {

  gaspi::initGaspiCxx();

  auto& comm_context = gaspi::getRuntime();
  gaspi::group::Group const group_all;
  gaspi::segment::Segment segment(1024*1024);

  std::unique_ptr<Field1D<float> > pFields[2]
    = { std::unique_ptr<Field1D<float> >( new Field1D<float>
          ( parameter::nInner1D
          , parameter::nBound1D
          , segment
          , comm_context
          , group_all ) )
      , std::unique_ptr<Field1D<float> >( new Field1D<float>
          ( parameter::nInner1D
          , parameter::nBound1D
          , segment
          , comm_context
          , group_all ) ) };

  for(int it(0)
     ;    it<parameter::nt
     ;  ++it ) {

    Field1D<float> const & uold(*pFields[(it+0)%2]);
    Field1D<float>       & unew(*pFields[(it+1)%2]);

    float const kappahat
      ( parameter::c * parameter::dt / parameter::dx
      * parameter::c * parameter::dt / parameter::dx );

    unew.updateLeftBoundary ( kappahat, uold );
    unew.updateRightBoundary( kappahat, uold );
    unew.updateInner( kappahat, uold);
    if(group_all.rank() == gaspi::group::Rank(2)) {
      float const nu (5.);
      float const T(1./nu);
      float const pi (std::atan(1.)*4.);
      float const sigmainv (2.*pi*nu);
      float const t (it * parameter::dt-T/2.);
      unew[10] = std::cos(+sigmainv * t)
               * std::exp(-sigmainv * sigmainv * t * t / 2.f);
    }
    unew.waitForHalos();
  }

  for(auto i(0UL);i<group_all.size();++i) {
    if(group_all.rank() == gaspi::group::Rank(i)) {
      for(int ix(0);ix<250;++ix) {
        std::cout << (*pFields[0])[4+ix] << std::endl;
      }
    }
    gaspi::getRuntime().barrier();
  }

  return EXIT_SUCCESS;
} catch(...) {
  return EXIT_FAILURE;
}
