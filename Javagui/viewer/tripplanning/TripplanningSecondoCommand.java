package viewer.tripplanning;

import java.util.ArrayList;
import java.util.List;

public class TripplanningSecondoCommand {

    String removeSource = "query EdgesHeight feed filter [.Source=-1] "
            + "EdgesHeight deletedirect consume;";
    String removeTarget = "query EdgesHeight feed filter [.Target=0] "
            + "EdgesHeight deletedirect consume;";
    String deleteSourcePos = "delete sourcePos;";
    String deleteMinSourceDist = "delete minSourceDist;";
    String deleteEdgesSourceDistances = "delete EdgesSourceDistances;";
    String deleteNearestPointSource = "delete nearestPointSource;";
    String deleteNearestPointSourcePos = "delete nearestPointSourcePos;";
    String deleteTargetPos = "delete targetPos;";
    String deleteEdgesTargetDistances = "delete EdgesTargetDistances;";
    String deleteMinTargetDist = "delete minTargetDist;";
    String deleteNearestPointTarget = "delete nearestPointTarget;";
    String deleteNearestPointTargetPos = "delete nearestPointTargetPos;";

    String setSourcePos = "let sourcePos=geocode (\"Katzbachstr\", 16, "
            + "10965, \"Berlin\");";

    String setNearestPointSource = "let nearestPointSource= EdgeIndex_Box_rtree EdgeIndex distancescan [sourcePos, 1] consume;";
    String setNearestPointSourcePos = "let nearestPointSourcePos= EdgesHeight feed filter [.Source=(nearestPointSource feed extract [Source])] extract [SourcePos];";

    String insertWayToSource = "query EdgesHeight inserttuple[-1, "
            + "(nearestPointSource feed extract [Source]), sourcePos, "
            + "(nearestPointSourcePos),1,2, (makesline(nearestPointSourcePos,sourcePos)),"
            + "[const text value ''],[const text "
            + "value ''],([const longint value 0]),[const lreal value "
            + "(((0.0 100.0 TRUE TRUE) (0.0 0.0)))]  ] consume;";

    String setTargetPos = "let targetPos =geocode (\"Methfesselstr\", 23,"
            + " 10965, \"Berlin\");";
    String setNearestPointTarget = "let nearestPointTarget= "
            + "EdgeIndex_Box_rtree EdgeIndex distancescan [targetPos, 1] consume;";
    String setNearestPointTargetPos = "let nearestPointTargetPos ="
            + "EdgesHeight feed filter [.Target=(nearestPointTarget feed extract [Target])] extract [TargetPos];";

    String insertWayToTarget = "query EdgesHeight inserttuple[( nearestPointTarget feed extract [Target]),0,  (nearestPointTargetPos),targetPos ,1,2, (makesline(nearestPointTargetPos,targetPos)),[const text value ''],[const text value ''],([const longint value -1]),[const lreal value (((0.0 100.0 TRUE TRUE) (0.0 0.0)))]  ] consume;";

   public TripplanningSecondoCommand() {
        super();
    }
    
    public TripplanningSecondoCommand(String sourceStreet, String sourceNo,
            String sourcePostcode, String sourceCity, String targetStreet, 
            String targetNo, String targetPostcode, String targetCity, double gradientWeight) {
        super();
        this.setSourcePos = "let sourcePos=geocode (\""+sourceStreet+"\", \""+
          sourceNo+"\","+sourcePostcode+",\""+sourceCity+"\");";
        this.setTargetPos = "let targetPos=geocode (\""+targetStreet+"\", \""+
                targetNo+"\","+targetPostcode+",\""+targetCity+"\");";
           
    }
    
    public List<String> getCommands() {

        List<String> commands = new ArrayList<String>();

        commands.add(removeSource);
        commands.add(removeTarget);
        commands.add(deleteTargetPos);
        deleteTempObjectsOnDatabase(commands);
        commands.add(setSourcePos);
        commands.add(setNearestPointSource);
        commands.add(setNearestPointSourcePos);
        commands.add(insertWayToSource);

        commands.add(setTargetPos);
        commands.add(setNearestPointTarget);
        commands.add(setNearestPointTargetPos);

        commands.add(insertWayToTarget);
        deleteTempObjectsOnDatabase(commands);

        return commands;
    }

    private void deleteTempObjectsOnDatabase(List<String> commands) {
        commands.add(deleteSourcePos);
        commands.add(deleteMinSourceDist);
        commands.add(deleteEdgesSourceDistances);
        commands.add(deleteNearestPointSource);
        commands.add(deleteNearestPointSourcePos);
        commands.add(deleteEdgesTargetDistances);
        commands.add(deleteMinTargetDist);
        commands.add(deleteNearestPointTarget);
        commands.add(deleteNearestPointTargetPos);
    }
}
