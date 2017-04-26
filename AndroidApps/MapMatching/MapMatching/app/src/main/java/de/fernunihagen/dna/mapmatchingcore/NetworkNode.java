package de.fernunihagen.dna.mapmatchingcore;

import java.util.ArrayList;
import java.util.List;

public class NetworkNode {
    private int nodeID;
    private List<NetworkEdge> outgoingEdges = new ArrayList<NetworkEdge>();

    public int getNodeID() {
        return nodeID;
    }

    public void setNodeID(int nodeID) {
        this.nodeID = nodeID;
    }

    public List<NetworkEdge> getOutgoingEdges() {
        return outgoingEdges;
    }
}
