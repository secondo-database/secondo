package ParallelSecondo;

import java.io.IOException;

import org.apache.hadoop.io.BooleanWritable;
import org.apache.hadoop.io.IntWritable;
import org.apache.hadoop.io.Text;
import org.apache.hadoop.mapreduce.Reducer;

import sj.lang.ListExpr;

public class PS_HadoopMap_Reduce 
	extends Reducer<Text, BooleanWritable, IntWritable, Text> 
	implements Constant
{

	boolean AgggregateResult = true;
	ListExpr resultList, last; 
	int cnt = 0;
	boolean createObj;
	
	@Override
	protected void reduce(Text key, Iterable<BooleanWritable> values,
			Context context) throws IOException,
			InterruptedException {
		
		
		for (BooleanWritable taskResult : values)
		{
			
			String[] rowInfo = key.toString().split(" ");
			
			int rowIdx 		= Integer.parseInt(rowInfo[0]);
			int slaveIdx 	= Integer.parseInt(rowInfo[1]);
			createObj = taskResult.get();
			AgggregateResult &= createObj;
			
			if (createObj)
			{
				resultList = ListExpr.concat(resultList, 
						ListExpr.twoElemList(
								ListExpr.intAtom(rowIdx), 
								ListExpr.intAtom(slaveIdx)));
				
				
/*				if (cnt == 0)
				{
					resultList = ListExpr.oneElemList(ListExpr.intAtom(slaveIdx));
					last = resultList;
				}
				else
				{
					last = ListExpr.append(last, ListExpr.intAtom(slaveIdx));
				}
*/				
				cnt++;
			}
		}
	}

	@Override
	protected void cleanup(Context context)
			throws IOException, InterruptedException {

		context.write(new IntWritable(cnt), 
			new Text(resultList.toString().
					replaceAll("\n", " ").replaceAll("\t", " ")));
	}
}
