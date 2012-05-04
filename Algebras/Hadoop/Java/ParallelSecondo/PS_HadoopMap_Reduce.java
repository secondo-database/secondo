package ParallelSecondo;

import java.io.IOException;

import org.apache.hadoop.io.BooleanWritable;
import org.apache.hadoop.io.IntWritable;
import org.apache.hadoop.io.Text;
import org.apache.hadoop.mapreduce.Reducer;

import sj.lang.ListExpr;

public class PS_HadoopMap_Reduce 
	extends Reducer<IntWritable, BooleanWritable, IntWritable, Text> 
	implements Constant
{

	boolean AgggregateResult = true;
	ListExpr resultList, last; 
	int cnt = 0;
	boolean createObj;
	
	@Override
	protected void reduce(IntWritable key, Iterable<BooleanWritable> values,
			Context context) throws IOException,
			InterruptedException {
		
		
		for (BooleanWritable taskResult : values)
		{
			
			int slaveIdx = key.get();
			createObj = taskResult.get();
			AgggregateResult &= createObj;
			
			if (createObj)
			{
				if (cnt == 0)
				{
					resultList = ListExpr.oneElemList(ListExpr.intAtom(slaveIdx));
					last = resultList;
				}
				else
				{
					last = ListExpr.append(last, ListExpr.intAtom(slaveIdx));
				}
				cnt++;
			}
		}
	}

	@Override
	protected void cleanup(Context context)
			throws IOException, InterruptedException {

		context.write(new IntWritable(0), 
			new Text(resultList.toString().
					replaceAll("\n", " ").replaceAll("\t", " ")));
	}
}
