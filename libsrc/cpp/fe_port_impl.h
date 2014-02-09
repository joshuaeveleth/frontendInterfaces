#ifndef FE_PORT_H
#define FE_PORT_H

#include "ossie/Port_impl.h"
#include <queue>
#include <list>
#include <boost/thread/condition_variable.hpp>
#include <boost/thread/locks.hpp>
#include <boost/make_shared.hpp>
#include <boost/ref.hpp>

#define CORBA_MAX_TRANSFER_BYTES omniORB::giopMaxMsgSize()

#include <ossie/CF/QueryablePort.h>

#include <FRONTEND/RFInfo.h>
#include <FRONTEND/GPS.h>
#include <FRONTEND/NavigationData.h>

#include <ossie/CorbaUtils.h>
#include "fe_base.h"
#include "fe_types.h"


namespace frontend {

    //
    // BEGIN FROM bulkio_p.h
    // used for boost shared pointer instantiation when user
    // supplied callback is provided
    //
    struct null_deleter
    {
        void operator()(void const *) const
        {
        }
    };
    // END FROM bulkio_p.h

    inline FRONTEND::RFInfoPkt* returnRFInfoPkt(const RFInfoPkt &val) {
        FRONTEND::RFInfoPkt* tmpVal = new FRONTEND::RFInfoPkt();
        tmpVal->rf_flow_id = CORBA::string_dup(val.rf_flow_id.c_str());
        tmpVal->rf_center_freq = val.rf_center_freq;
        tmpVal->rf_bandwidth = val.rf_bandwidth;
        tmpVal->if_center_freq = val.if_center_freq;
        tmpVal->spectrum_inverted = val.spectrum_inverted;
        tmpVal->sensor.collector = CORBA::string_dup(val.sensor.collector.c_str());
        tmpVal->sensor.mission = CORBA::string_dup(val.sensor.mission.c_str());
        tmpVal->sensor.rx = CORBA::string_dup(val.sensor.rx.c_str());
        tmpVal->sensor.antenna.description = CORBA::string_dup(val.sensor.antenna.description.c_str());
        tmpVal->sensor.antenna.name = CORBA::string_dup(val.sensor.antenna.name.c_str());
        tmpVal->sensor.antenna.size = CORBA::string_dup(val.sensor.antenna.size.c_str());
        tmpVal->sensor.antenna.type = CORBA::string_dup(val.sensor.antenna.type.c_str());
        tmpVal->sensor.feed.name = CORBA::string_dup(val.sensor.feed.name.c_str());
        tmpVal->sensor.feed.polarization = CORBA::string_dup(val.sensor.feed.polarization.c_str());
        tmpVal->sensor.feed.freq_range.max_val = val.sensor.feed.freq_range.max_val;
        tmpVal->sensor.feed.freq_range.min_val = val.sensor.feed.freq_range.min_val;
        tmpVal->sensor.feed.freq_range.values.length(val.sensor.feed.freq_range.values.size());
        for (unsigned int i=0; i<val.sensor.feed.freq_range.values.size(); i++) {
            tmpVal->sensor.feed.freq_range.values[i] = val.sensor.feed.freq_range.values[i];
        }
        return tmpVal;
    };
    inline RFInfoPkt returnRFInfoPkt(const FRONTEND::RFInfoPkt &tmpVal) {
        RFInfoPkt val;
        val.rf_flow_id = ossie::corba::returnString(tmpVal.rf_flow_id);
        val.rf_center_freq = tmpVal.rf_center_freq;
        val.rf_bandwidth = tmpVal.rf_bandwidth;
        val.if_center_freq = tmpVal.if_center_freq;
        val.spectrum_inverted = tmpVal.spectrum_inverted;
        val.sensor.collector = ossie::corba::returnString(tmpVal.sensor.collector);
        val.sensor.mission = ossie::corba::returnString(tmpVal.sensor.mission);
        val.sensor.rx = ossie::corba::returnString(tmpVal.sensor.rx);
        val.sensor.antenna.description = ossie::corba::returnString(tmpVal.sensor.antenna.description);
        val.sensor.antenna.name = ossie::corba::returnString(tmpVal.sensor.antenna.name);
        val.sensor.antenna.size = ossie::corba::returnString(tmpVal.sensor.antenna.size);
        val.sensor.antenna.type = ossie::corba::returnString(tmpVal.sensor.antenna.type);
        val.sensor.feed.name = ossie::corba::returnString(tmpVal.sensor.feed.name);
        val.sensor.feed.polarization = ossie::corba::returnString(tmpVal.sensor.feed.polarization);
        val.sensor.feed.freq_range.max_val = tmpVal.sensor.feed.freq_range.max_val;
        val.sensor.feed.freq_range.min_val = tmpVal.sensor.feed.freq_range.min_val;
        val.sensor.feed.freq_range.values.resize(tmpVal.sensor.feed.freq_range.values.length());
        for (unsigned int i=0; i<val.sensor.feed.freq_range.values.size(); i++) {
            val.sensor.feed.freq_range.values[i] = tmpVal.sensor.feed.freq_range.values[i];
        }
        return val;
    };

    //
    // Callback signatures to register when functions are called (provides ports only)
    //

    // for InGPSPort
    typedef FRONTEND::GPSInfo* (*GPSInfoFromVoidFn)( void );
    typedef void (*VoidFromGPSInfoFn)( const FRONTEND::GPSInfo& data );
    typedef FRONTEND::GpsTimePos* (*GpsTimePosFromVoidFn)( void );
    typedef void (*VoidFromGpsTimePosFn)( const FRONTEND::GpsTimePos& data );

    // for InNavDataPort
    typedef FRONTEND::NavigationPacket* (*NavPktFromVoidFn)( void );
    typedef void (*VoidFromNavPktFn)( const FRONTEND::NavigationPacket& data );


    // for InGPSPort
    class GPSInfoFromVoid {
        public:
            virtual FRONTEND::GPSInfo* operator() ( void ) = 0;
            virtual ~GPSInfoFromVoid() {};
    };
    class GpsTimePosFromVoid {
        public:
            virtual FRONTEND::GpsTimePos* operator() ( void ) = 0;
            virtual ~GpsTimePosFromVoid() {};
    };
    class VoidFromGPSInfo {
        public:
            virtual void operator() ( const FRONTEND::GPSInfo& data ) = 0;
            virtual ~VoidFromGPSInfo() {};
    };
    class VoidFromGpsTimePos {
        public:
            virtual void operator() ( const FRONTEND::GpsTimePos& data ) = 0;
            virtual ~VoidFromGpsTimePos() {};
    };

    // for InNavDataPort
    class NavPktFromVoid {
        public:
            virtual FRONTEND::NavigationPacket* operator() ( void ) = 0;
            virtual ~NavPktFromVoid() {};
    };

    class VoidFromNavPkt {
        public:
            virtual void operator() ( const FRONTEND::NavigationPacket& data ) = 0;
            virtual ~VoidFromNavPkt() {};
    };

    /**
    * Allow for member functions to be used as callbacks
    */

    // for InGPSPort
    template <class T>
    class MemberGPSInfoFromVoid : public GPSInfoFromVoid
    {
        public:
            typedef boost::shared_ptr< MemberGPSInfoFromVoid< T > > SPtr;
            typedef FRONTEND::GPSInfo* (T::*MemberGPSInfoFromVoidFn)( void );
            static SPtr Create( T &target, MemberGPSInfoFromVoidFn func ){
                return SPtr( new MemberGPSInfoFromVoid(target, func ) );
            };
            virtual FRONTEND::GPSInfo* operator() ( void )
            {
                return (target_.*func_)();
            }
            MemberGPSInfoFromVoid ( T& target,  MemberGPSInfoFromVoidFn func) :
                target_(target),
                func_(func)
                  {
                  }
        private:
            T& target_;
            MemberGPSInfoFromVoidFn func_;
    };
    template <class T>
    class MemberGpsTimePosFromVoid : public GpsTimePosFromVoid
    {
        public:
            typedef boost::shared_ptr< MemberGpsTimePosFromVoid< T > > SPtr;
            typedef FRONTEND::GpsTimePos* (T::*MemberGpsTimePosFromVoidFn)( void );
            static SPtr Create( T &target, MemberGpsTimePosFromVoidFn func ){
                return SPtr( new MemberGpsTimePosFromVoid(target, func ) );
            };
            virtual FRONTEND::GpsTimePos* operator() ( void )
            {
                return (target_.*func_)();
            }
            MemberGpsTimePosFromVoid ( T& target,  MemberGpsTimePosFromVoidFn func) :
                target_(target),
                func_(func)
                  {
                  }
        private:
            T& target_;
            MemberGpsTimePosFromVoidFn func_;
    };
    template <class T>
    class MemberVoidFromGPSInfo : public VoidFromGPSInfo
    {
        public:
            typedef boost::shared_ptr< MemberVoidFromGPSInfo< T > > SPtr;
            typedef void (T::*MemberVoidFromGPSInfoFn)( const FRONTEND::GPSInfo& data );
            static SPtr Create( T &target, MemberVoidFromGPSInfoFn func ){
                return SPtr( new MemberVoidFromGPSInfo(target, func ) );
            };
            virtual void operator() (const FRONTEND::GPSInfo& data )
            {
                (target_.*func_)(data);
            }
            MemberVoidFromGPSInfo ( T& target,  MemberVoidFromGPSInfoFn func) :
                target_(target),
                func_(func)
                  {
                  }
        private:
            T& target_;
            MemberVoidFromGPSInfoFn func_;
    };
    template <class T>
    class MemberVoidFromGpsTimePos : public VoidFromGpsTimePos
    {
        public:
            typedef boost::shared_ptr< MemberVoidFromGpsTimePos< T > > SPtr;
            typedef void (T::*MemberVoidFromGpsTimePosFn)( const FRONTEND::GpsTimePos& data );
            static SPtr Create( T &target, MemberVoidFromGpsTimePosFn func ){
                return SPtr( new MemberVoidFromGpsTimePos(target, func ) );
            };
            virtual void operator() (const FRONTEND::GpsTimePos& data )
            {
                (target_.*func_)(data);
            }
            MemberVoidFromGpsTimePos ( T& target,  MemberVoidFromGpsTimePosFn func) :
                target_(target),
                func_(func)
                  {
                  }
        private:
            T& target_;
            MemberVoidFromGpsTimePosFn func_;
    };

    // for InNavDataPort
    template <class T>
    class MemberNavPktFromVoid : public NavPktFromVoid
    {
        public:
            typedef boost::shared_ptr< MemberNavPktFromVoid< T > > SPtr;
            typedef FRONTEND::NavigationPacket* (T::*MemberNavPktFromVoidFn)( void );
            static SPtr Create( T &target, MemberNavPktFromVoidFn func ){
                return SPtr( new MemberNavPktFromVoid(target, func ) );
            };
            virtual FRONTEND::NavigationPacket* operator() ( void )
            {
                return (target_.*func_)();
            }
            MemberNavPktFromVoid ( T& target,  MemberNavPktFromVoidFn func) :
                target_(target),
                func_(func)
                  {
                  }
        private:
            T& target_;
            MemberNavPktFromVoidFn func_;
    };
    template <class T>
    class MemberVoidFromNavPkt : public VoidFromNavPkt
    {
        public:
            typedef boost::shared_ptr< MemberVoidFromNavPkt< T > > SPtr;
            typedef void (T::*MemberVoidFromNavPktFn)( const FRONTEND::NavigationPacket& data );
            static SPtr Create( T &target, MemberVoidFromNavPktFn func ){
                return SPtr( new MemberVoidFromNavPkt(target, func ) );
            };
            virtual void operator() (const FRONTEND::NavigationPacket& data )
            {
                (target_.*func_)(data);
            }
            MemberVoidFromNavPkt ( T& target,  MemberVoidFromNavPktFn func) :
                target_(target),
                func_(func)
                  {
                  }
        private:
            T& target_;
            MemberVoidFromNavPktFn func_;
    };

    /**
    * Wrap Callback functions as CB objects
    */

    // for InGPSPort
    class StaticGPSInfoFromVoid : public GPSInfoFromVoid
    {
    public:
        virtual FRONTEND::GPSInfo* operator() ( void )
        {
            return (*func_)();
        }
        StaticGPSInfoFromVoid ( GPSInfoFromVoidFn func) :
            func_(func)
        {
        }
    private:
        GPSInfoFromVoidFn func_;
    };
    class StaticGpsTimePosFromVoid : public GpsTimePosFromVoid
    {
    public:
        virtual FRONTEND::GpsTimePos* operator() ( void )
        {
            return (*func_)();
        }
        StaticGpsTimePosFromVoid ( GpsTimePosFromVoidFn func) :
            func_(func)
        {
        }
    private:
        GpsTimePosFromVoidFn func_;
    };
    class StaticVoidFromGPSInfo : public VoidFromGPSInfo
    {
    public:
        virtual void operator() ( const FRONTEND::GPSInfo& data)
        {
            return (*func_)(data);
        }
        StaticVoidFromGPSInfo ( VoidFromGPSInfoFn func) :
            func_(func)
        {
        }
    private:
        VoidFromGPSInfoFn func_;
    };
    class StaticVoidFromGpsTimePos : public VoidFromGpsTimePos
    {
    public:
        virtual void operator() ( const FRONTEND::GpsTimePos& data)
        {
            return (*func_)(data);
        }
        StaticVoidFromGpsTimePos ( VoidFromGpsTimePosFn func) :
            func_(func)
        {
        }
    private:
        VoidFromGpsTimePosFn func_;
    };

    // for InNavDataPort
    class StaticNavPktFromVoid : public NavPktFromVoid
    {
    public:
        virtual FRONTEND::NavigationPacket* operator() ( void )
        {
            return (*func_)();
        }
        StaticNavPktFromVoid ( NavPktFromVoidFn func) :
            func_(func)
        {
        }
    private:
        NavPktFromVoidFn func_;
    };
    class StaticVoidFromNavPkt : public VoidFromNavPkt
    {
    public:
        virtual void operator() ( const FRONTEND::NavigationPacket& data)
        {
            return (*func_)(data);
        }
        StaticVoidFromNavPkt ( VoidFromNavPktFn func) :
            func_(func)
        {
        }
    private:
        VoidFromNavPktFn func_;
    };

    // ----------------------------------------------------------------------------------------
    // OutFrontendPort declaration
    // ----------------------------------------------------------------------------------------
    template <typename PortType_var, typename PortType>
    class OutFrontendPort : public Port_Uses_base_impl, public POA_ExtendedCF::QueryablePort
    {
        public:
            OutFrontendPort(std::string port_name) :
                Port_Uses_base_impl(port_name)
            {
                recConnectionsRefresh = false;
                recConnections.length(0);
            }
            OutFrontendPort(std::string port_name,
                                        LOGGER_PTR logger) :
                Port_Uses_base_impl(port_name),
                                logger(logger)
            {
                recConnectionsRefresh = false;
                recConnections.length(0);
            }
            ~OutFrontendPort(){
            }

                void setLogger(LOGGER_PTR newLogger){
                    logger = newLogger;
                }

            ExtendedCF::UsesConnectionSequence * connections()
            {
                boost::mutex::scoped_lock lock(updatingPortsLock);   // don't want to process while command information is coming in
                if (recConnectionsRefresh) {
                    recConnections.length(outConnections.size());
                    for (unsigned int i = 0; i < outConnections.size(); i++) {
                        recConnections[i].connectionId = CORBA::string_dup(outConnections[i].second.c_str());
                        recConnections[i].port = CORBA::Object::_duplicate(outConnections[i].first);
                    }
                    recConnectionsRefresh = false;
                }
                ExtendedCF::UsesConnectionSequence_var retVal = new ExtendedCF::UsesConnectionSequence(recConnections);
                // NOTE: You must delete the object that this function returns!
                return retVal._retn();
            };

            void connectPort(CORBA::Object_ptr connection, const char* connectionId)
            {
                boost::mutex::scoped_lock lock(updatingPortsLock);   // don't want to process while command information is coming in
                PortType_var port = PortType::_narrow(connection);
                outConnections.push_back(std::make_pair(port, connectionId));
                active = true;
                recConnectionsRefresh = true;
            };

            void disconnectPort(const char* connectionId)
            {
                boost::mutex::scoped_lock lock(updatingPortsLock);   // don't want to process while command information is coming in
                for (unsigned int i = 0; i < outConnections.size(); i++) {
                    if (outConnections[i].second == connectionId) {
                        outConnections.erase(outConnections.begin() + i);
                        break;
                    }
                }

                if (outConnections.size() == 0) {
                    active = false;
                }
                recConnectionsRefresh = true;
            };

            std::vector< std::pair<PortType_var, std::string> > _getConnections()
            {
                return outConnections;
            };

        protected:
            std::vector < std::pair<PortType_var, std::string> > outConnections;
            ExtendedCF::UsesConnectionSequence recConnections;
            bool recConnectionsRefresh;

                LOGGER_PTR logger;
    };

} // end of frontend namespace


#endif
