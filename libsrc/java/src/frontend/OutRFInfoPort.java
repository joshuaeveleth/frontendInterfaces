package frontend;

import org.apache.log4j.Logger;
import java.util.HashMap;
import java.util.Map;
import org.ossie.component.QueryableUsesPort;
import FRONTEND.RFInfoOperations;
import FRONTEND.RFInfoHelper;
import FRONTEND.RFInfoPkt;


public class OutRFInfoPort extends QueryableUsesPort<RFInfoOperations> implements RFInfoOperations {

    protected String name;
 
    protected Logger logger = null;

    protected Object updatingPortsLock;

    /**
     * Map of connection Ids to port objects
     */
    protected Map<String, RFInfoOperations> outConnections = new HashMap<String, RFInfoOperations>();

    public OutRFInfoPort( String portName) {
        this( portName, null);
    }

    public OutRFInfoPort( String portName,
                         Logger logger) {
        super(portName);
        this.name = portName;
        this.logger = logger;
        this.outConnections = new HashMap<String, RFInfoOperations>();
    }

    protected RFInfoOperations narrow(org.omg.CORBA.Object connection) {
        RFInfoOperations ops = RFInfoHelper.narrow(connection);
        return ops;
    }

    public void connectPort(final org.omg.CORBA.Object connection, final String connectionId) throws CF.PortPackage.InvalidPort, CF.PortPackage.OccupiedPort {
        try {
            synchronized (this.updatingPortsLock) {
                super.connectPort(connection, connectionId);
                final RFInfoOperations port = RFInfoHelper.narrow(connection);
                this.outConnections.put(connectionId, port);
                this.active = true;
            }
        } catch (final Throwable t) {
            t.printStackTrace();
        }

    }

    public void disconnectPort(final String connectionId) throws CF.PortPackage.InvalidPort {
        synchronized (this.updatingPortsLock) {
            super.disconnectPort(connectionId);
            this.outConnections.remove(connectionId);
            this.active = (this.outConnections.size() != 0);
        }
    }

    public String rf_flow_id() {
        String retval = "";

        synchronized(updatingPortsLock){
            if (this.active) {
                for (RFInfoOperations p : this.outConnections.values()) {
                    retval = p.rf_flow_id();
                }
            }
        }
        return retval;
    }

    public void rf_flow_id(String data) {
        synchronized(updatingPortsLock){
            if (this.active) {
                for (RFInfoOperations p : this.outConnections.values()) {
                    p.rf_flow_id(data);
                }
            }
        }
    }

    public RFInfoPkt rfinfo_pkt() {
        RFInfoPkt retval = null;

        synchronized(updatingPortsLock){
            if (this.active) {
                for (RFInfoOperations p : this.outConnections.values()) {
                    retval = p.rfinfo_pkt();
                }
            }
        }
        return retval;
    }

    public void rfinfo_pkt(RFInfoPkt data) {
        synchronized(updatingPortsLock){
            if (this.active) {
                for (RFInfoOperations p : this.outConnections.values()) {
                    p.rfinfo_pkt(data);
                }
            }
        }
    }

    public void setLogger( Logger newLogger ) {
        logger = newLogger;
    }
}

