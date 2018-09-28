package mmdb.streamprocessing.objectnodes.projection;

import mmdb.data.MemoryObject;
import mmdb.data.attributes.moving.AttributeMpoint;
import mmdb.data.attributes.spatial.AttributeLine;
import mmdb.data.attributes.util.SpatialObjects.Segment;
import mmdb.error.memory.MemoryException;
import mmdb.error.streamprocessing.ParsingException;
import mmdb.error.streamprocessing.TypeException;
import mmdb.streamprocessing.Node;
import mmdb.streamprocessing.Nodes;
import mmdb.streamprocessing.objectnodes.ObjectNode;
import mmdb.streamprocessing.parser.Environment;
import mmdb.streamprocessing.parser.NestedListProcessor;
import mmdb.streamprocessing.tools.ParserTools;
import mmdb.streamprocessing.tools.TypecheckTools;
import sj.lang.ListExpr;
import stlib.interfaces.spatial.LineIF;
import stlib.interfaces.spatial.util.HalfsegmentIF;

/**
 *
 * 
 * @author Markus Fuessel
 */
public class Trajectory implements ObjectNode {

   /**
    * Operator parameter Node
    */
   private Node input;

   /**
    * Operator parameter as ObjectNode
    */
   private ObjectNode objectInput;

   /**
    * The operator's output type.
    */
   private MemoryObject outputType;

   /**
    * @see mmdb.streamprocessing.Nodes#fromNL(ListExpr[], Environment)
    */
   public static Node fromNL(ListExpr[] params, Environment environment) throws ParsingException {
      ParserTools.checkListElemCount(params, 1, Trajectory.class);
      Node node1 = NestedListProcessor.nlToNode(params[0], environment);
      return new Trajectory(node1);
   }

   /**
    * Constructor, called by fromNL(...)
    * 
    * @param input
    *           operator's first parameter
    * 
    */
   public Trajectory(final Node input) {
      this.input = input;

   }

   /**
    * {@inheritDoc}
    */
   @Override
   public void typeCheck() throws TypeException {
      this.input.typeCheck();

      // Is input1 an ObjectNode?
      TypecheckTools.checkNodeType(this.input, ObjectNode.class, this.getClass(), 1);
      this.objectInput = (ObjectNode) this.input;

      if (this.objectInput.getOutputType().getClass() == AttributeMpoint.class) {
         this.outputType = new AttributeLine();

      } else {
         throw new TypeException("%s's %ss input needs to provide %s, but provides %s.",
               this.getClass().getSimpleName(), Nodes.NodeType.ObjectNode, AttributeMpoint.class.getSimpleName(),
               objectInput.getClass().getSimpleName());
      }

   }

   /**
    * {@inheritDoc}
    */
   @Override
   public MemoryObject getOutputType() {
      return this.outputType;
   }

   /**
    * {@inheritDoc}
    */
   @Override
   public MemoryObject getResult() throws MemoryException {

      AttributeMpoint mpoint = (AttributeMpoint) this.objectInput.getResult();

      System.out.println(mpoint.getNoUnits());

      LineIF line = stlib.operations.projection.Trajectory.execute(mpoint);

      AttributeLine lineMMDB = new AttributeLine();

      System.out.println(line.getHalfsegments().size());

      for (HalfsegmentIF hs : line.getHalfsegments()) {
         if (hs.isLeftDominating()) {
            Segment seg = new Segment();

            seg.setxValue1((float) hs.getLeftPoint().getXValue());
            seg.setyValue1((float) hs.getLeftPoint().getYValue());
            seg.setxValue2((float) hs.getRightPoint().getXValue());
            seg.setyValue2((float) hs.getRightPoint().getYValue());

            lineMMDB.addSegment(seg);

         }
      }

      return lineMMDB;
   }

}
