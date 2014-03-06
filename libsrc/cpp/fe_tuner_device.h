#ifndef FE_TUNER_DEVICE_BASE_H
#define FE_TUNER_DEVICE_BASE_H

#include <ossie/Device_impl.h>
#include <uuid/uuid.h>
#include <redhawk/FRONTEND/Frontend.h>

#include "ossie/prop_helpers.h"
#include "bulkio/bulkio.h"
#include "fe_tuner_struct_props.h"
#include "fe_tuner_port_impl.h"
#include "fe_rfinfo_port_impl.h"
#include "fe_rfsource_port_impl.h"

/*********************************************************************************************/
/**************************              FRONTEND                   **************************/
/*********************************************************************************************/
namespace frontend {

    /** Individual Tuner. This structure contains stream specific data for channel/tuner to include:
     *         - Additional stream metadata (sri)
     *         - Control information (allocation id's)
     *         - Reference to associated frontend_tuner_status property where additional information is held. Note: frontend_tuner_status structure is required by frontend interfaces v2.0
     */
    
    inline std::string new_uuid() {
        uuid_t new_random_uuid;
        uuid_generate_random(new_random_uuid);
        char new_random_uuid_str[37];
        uuid_unparse(new_random_uuid, new_random_uuid_str);
        return std::string(new_random_uuid_str);
    };
    
    template < typename TunerStatusStructType >
    struct indivTuner {
        indivTuner(){
            frontend_status = NULL;
            lock = NULL;
        }
        boost::mutex *lock;
        std::string control_allocation_id;
        TunerStatusStructType* frontend_status;

        void reset(){
            control_allocation_id.clear();
            if(frontend_status != NULL){
                frontend_status->allocation_id_csv.clear();
                frontend_status->center_frequency = 0.0;
                frontend_status->bandwidth = 0.0;
                frontend_status->sample_rate = 0.0;
                frontend_status->enabled = false;
            }
        }
    };

    //////////////////////////////////
    //    TUNER CLASS DEFINITION    //
    //////////////////////////////////

    template < typename TunerStatusStructType >
    class FrontendTunerDevice : public Device_impl
    {
        public:
            FrontendTunerDevice(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl);
            FrontendTunerDevice(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl, char *compDev);
            FrontendTunerDevice(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl, CF::Properties capacities);
            FrontendTunerDevice(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl, CF::Properties capacities, char *compDev);
            ~FrontendTunerDevice();

            // this is implemented in the generated base class once all properties are known
            virtual void loadProperties();

            // Device specific allocation handling
            virtual CF::Device::UsageType updateUsageState();
            virtual CORBA::Boolean allocateCapacity(const CF::Properties & capacities) throw (CORBA::SystemException, CF::Device::InvalidCapacity, CF::Device::InvalidState);
            virtual void deallocateCapacity(const CF::Properties & capacities)throw (CORBA::SystemException, CF::Device::InvalidCapacity, CF::Device::InvalidState);

            // Mapping and translation helpers. External string identifiers to internal numerical identifiers
            virtual long addTunerMapping(const frontend::frontend_tuner_allocation_struct & frontend_alloc);
            virtual long addListenerMapping(const frontend::frontend_listener_allocation_struct & frontend_listener_alloc);
            virtual bool removeTunerMapping(std::string allocation_id);
            virtual bool removeTunerMapping(size_t tuner_id);
            virtual long getTunerMapping(std::string allocation_id);
            virtual bool is_connectionID_valid_for_tunerID( const size_t & tuner_id, const std::string & connectionID);
            virtual bool is_connectionID_valid_for_streamID( const std::string & streamID, const std::string & connectionID);
            virtual bool is_connectionID_controller_for_streamID(const std::string & streamID, const std::string & connectionID);
            virtual bool is_connectionID_listener_for_streamID(const std::string & streamID, const std::string & connectionID);
            virtual bool is_freq_valid(double req_cf, double req_bw, double req_sr, double cf, double bw, double sr);
            virtual void assignListener(std::string& listen_alloc_id, std::string& alloc_id);
            virtual void removeListener(std::string& listen_alloc_id);

            // Configure tuner - gets called during allocation
            virtual bool enableTuner(size_t tuner_id, bool enable); /* assumes collector RF and channel RF are the same. If not true, override function */
            virtual bool removeTuner(size_t tuner_id);
            
            BULKIO::StreamSRI create(std::string &stream_id, TunerStatusStructType &frontend_status, double collector_frequency = -1.0) {
                BULKIO::StreamSRI sri;
                sri.hversion = 1;
                sri.xstart = 0.0;
                if ( frontend_status.sample_rate <= 0.0 )
                    sri.xdelta =  1.0;
                else
                    sri.xdelta = 1/frontend_status.sample_rate;
                sri.xunits = BULKIO::UNITS_TIME;
                sri.subsize = 0;
                sri.ystart = 0.0;
                sri.ydelta = 0.0;
                sri.yunits = BULKIO::UNITS_NONE;
                sri.mode = 0;
                sri.blocking=false;
                sri.streamID = stream_id.c_str();
                long colFreq;
                if (collector_frequency < 0)
                    colFreq = frontend_status.center_frequency;
                else
                    colFreq = long(collector_frequency);
                this->addModifyKeyword<CORBA::Double > (&sri, "COL_RF", CORBA::Double(colFreq));
                this->addModifyKeyword<CORBA::Double > (&sri, "CHAN_RF", CORBA::Double(frontend_status.center_frequency));
                this->addModifyKeyword<std::string> (&sri,"FRONTEND::RF_FLOW_ID",frontend_status.rf_flow_id);
                this->addModifyKeyword<CORBA::Double> (&sri,"FRONTEND::BANDWIDTH", CORBA::Double(frontend_status.bandwidth));
                this->addModifyKeyword<std::string> (&sri,"FRONTEND::DEVICE_ID",std::string(identifier()));
                return sri;
            }
            
        protected:
            typedef std::map<std::string, size_t> string_number_mapping;
            typedef boost::mutex::scoped_lock exclusive_lock;

            // Member variables exposed as properties
            std::string device_kind;
            std::string device_model;
            frontend::frontend_tuner_allocation_struct frontend_tuner_allocation;
            frontend::frontend_listener_allocation_struct frontend_listener_allocation;
            std::vector<TunerStatusStructType> frontend_tuner_status;

            std::string create_allocation_id_csv(size_t tuner_id);

            // tunerChannels is exclusively paired with property tuner_status.
            // tunerChannels provide stream information for the channel while tuner_status provides the tuner information.
            std::vector<frontend::indivTuner<TunerStatusStructType> > tunerChannels;

            // Provides mapping from unique allocation ID to internal tuner (channel) number
            string_number_mapping allocationID_to_tunerID;
            string_number_mapping streamID_to_tunerID;
            boost::mutex allocationID_MappingLock;

            ///////////////////////////////
            // Device specific functions // -- virtual - to be implemented by device developer
            ///////////////////////////////
            virtual bool _dev_enable(size_t tuner_id) = 0;
            virtual bool _dev_disable(size_t tuner_id) = 0;

            virtual bool _dev_set_tuning(std::string &tuner_type, tuning_request &request, size_t tuner_id) = 0;
            virtual bool _dev_del_tuning(size_t tuner_id) = 0;
            virtual void removeAllocationIdRouting(const std::string allocation_id) = 0;

            ////////////////////////////
            // Other helper functions //
            ////////////////////////////

            virtual double optimize_rate(const double& req_rate, const double& max_rate, const double& min_rate){
                //for(size_t dec = size_t(max_rate/min_rate); dec >= 1; dec--){
                //    if(max_rate/double(dec) >= req_rate)
                //        return max_rate/double(dec);
                //}
                if(req_rate < min_rate)
                    return min_rate;
                return req_rate;
            }

            template <typename CORBAXX> bool addModifyKeyword(BULKIO::StreamSRI *sri, CORBA::String_member id, CORBAXX myValue, bool addOnly = false) {
                CORBA::Any value;
                value <<= myValue;
                unsigned long keySize = sri->keywords.length();
                if (!addOnly) {
                    for (unsigned int i = 0; i < keySize; i++) {
                        if (!strcmp(sri->keywords[i].id, id)) {
                            sri->keywords[i].value = value;
                            return true;
                        }
                    }
                }
                sri->keywords.length(keySize + 1);
                if (sri->keywords.length() != keySize + 1)
                    return false;
                sri->keywords[keySize].id = CORBA::string_dup(id);
                sri->keywords[keySize].value = value;
                return true;
            }

            // This is not currently used but is available as a debugging tool
            void printSRI(BULKIO::StreamSRI *sri, std::string strHeader = "DEBUG SRI"){
                std::cout << strHeader << ":\n";
                std::cout << "\thversion: " << sri->hversion<< std::endl;
                std::cout << "\txstart: " << sri->xstart<< std::endl;
                std::cout << "\txdelta: " << sri->xdelta<< std::endl;
                std::cout << "\txunits: " << sri->xunits<< std::endl;
                std::cout << "\tsubsize: " << sri->subsize<< std::endl;
                std::cout << "\tystart: " << sri->ystart<< std::endl;
                std::cout << "\tydelta: " << sri->ydelta<< std::endl;
                std::cout << "\tyunits: " << sri->yunits<< std::endl;
                std::cout << "\tmode: " << sri->mode<< std::endl;
                std::cout << "\tstreamID: " << sri->streamID<< std::endl;
                unsigned long keySize = sri->keywords.length();
                for (unsigned int i = 0; i < keySize; i++) {
                    std::cout << "\t KEYWORD KEY/VAL :: " << sri->keywords[i].id << ": " << ossie::any_to_string(sri->keywords[i].value) << std::endl;
                }
                std::cout << std::endl;
            }

        private:
            // this will be overridden by the generated base class once all ports are known
            virtual void construct();
    };

}; // end frontend namespace

#endif
