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
    String setEdgesSourcesDist = "let EdgesSourceDistances=Edges feed "
            + "extend [Dist:fun(t:TUPLE) distance(attr(t,SourcePos), "
            + "sourcePos)] consume;";
    String setMinSourceDist = "let minSourceDist = EdgesSourceDistances "
            + "feed min[Dist];";
    String setNearestPointSource = "let nearestPointSource ="
            + "EdgesSourceDistances feed filter[.Dist=minSourceDist] consume;";
    String setNearestPointSourcePos = "let nearestPointSourcePos ="
            + "nearestPointSource feed extract [SourcePos];";

    String insertWayToSource = "query EdgesHeight inserttuple[-1, "
            + "(nearestPointSource feed extract [Source]), sourcePos, "
            + "(nearestPointSourcePos),1,2, (makesline(sourcePos, "
            + "nearestPointSourcePos)),[const text value ''],[const text "
            + "value ''],([const longint value 0]),[const lreal value "
            + "(((0.0 100.0 TRUE TRUE) (0.0 0.0)))]  ] consume;";

    String setTargetPos = "let targetPos =geocode (\"Methfesselstr\", 23,"
            + " 10965, \"Berlin\");";
    String setEdgesTargetDistances = "let EdgesTargetDistances =Edges feed"
            + " extend [Dist:fun(t:TUPLE) distance(attr(t,TargetPos), "
            + "targetPos)] consume;";
    String setMinTargetDist = "let minTargetDist = EdgesTargetDistances"
            + " feed min[Dist];";
    String setNearestPointTarget = "let nearestPointTarget= "
            + "EdgesTargetDistances feed filter[.Dist= minTargetDist ] "
            + "consume;";
    String setNearestPointTargetPos = "let nearestPointTargetPos ="
            + "nearestPointTarget feed extract [TargetPos];";

    String insertWayToTarget = "query EdgesHeight inserttuple[( nearestPointTarget feed extract [Target]),0,  (nearestPointTargetPos),targetPos ,1,2, (makesline(nearestPointTargetPos,targetPos)),[const text value ''],[const text value ''],([const longint value -1]),[const lreal value (((0.0 100.0 TRUE TRUE) (0.0 0.0)))]  ] consume;";

  //  String queryTrip = "query EdgesHeight oshortestpatha[-1,0,0;distanceWithGradient(.SourcePos,.TargetPos,150.5,.Heightfunction), distance(.TargetPos, targetPos)] feed extend[Gradient: (lfResult(size(.Curve), .Heightfunction) -  lfResult(0.0, .Heightfunction)) / size(gk(.Curve))] consume;";

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
        
      //  this.queryTrip = "query EdgesHeight oshortestpatha[-1,0,0;distanceWithGradient(.SourcePos,.TargetPos,"+gradientWeight+",.Heightfunction), distance(.TargetPos, targetPos)] feed extend[Gradient: (lfResult(size(.Curve), .Heightfunction) -  lfResult(0.0, .Heightfunction)) / size(gk(.Curve))] consume;";
             
    }
    
    public List<String> getCommands() {

        List<String> commands = new ArrayList<String>();

        commands.add(removeSource);
        commands.add(removeTarget);
        commands.add(deleteTargetPos);
        deleteTempObjectsOnDatabase(commands);
        commands.add(setSourcePos);
        commands.add(setEdgesSourcesDist);
        commands.add(setMinSourceDist);
        commands.add(setNearestPointSource);
        commands.add(setNearestPointSourcePos);
        commands.add(insertWayToSource);

        commands.add(setTargetPos);
        commands.add(setEdgesTargetDistances);
        commands.add(setMinTargetDist);
        commands.add(setNearestPointTarget);
        commands.add(setNearestPointTargetPos);

        commands.add(insertWayToTarget);
     //   commands.add(queryTrip);
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
