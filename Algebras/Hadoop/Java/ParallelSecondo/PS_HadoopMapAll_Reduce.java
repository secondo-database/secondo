package ParallelSecondo;

import java.io.IOException;

import org.apache.hadoop.io.IntWritable;
import org.apache.hadoop.io.Text;
import org.apache.hadoop.mapreduce.Reducer;

import sj.lang.ListExpr;

public class PS_HadoopMapAll_Reduce 
	extends	Reducer<Text, Text, IntWritable, Text> 
	implements Constant
{
	
	ListExpr resultList;
	int cnt = 0;
	

	/* (non-Javadoc)
	 * @see org.apache.hadoop.mapreduce.Reducer#reduce(java.lang.Object, java.lang.Iterable, org.apache.hadoop.mapreduce.Reducer.Context)
	 */
	@Override
	protected void reduce(Text key, Iterable<Text> values,
			Context context) throws IOException,
			InterruptedException {

		for (Text value : values)
		{
			String[] rowInfo = key.toString().split(" ");
			int rowIdx = Integer.parseInt(rowInfo[0]);
			int slaveIdx = Integer.parseInt(rowInfo[1]);

			ListExpr taskResult = new ListExpr();
			taskResult.readFromString(value.toString());
			boolean succ = taskResult.first().boolValue();
			
			resultList = ListExpr.concat(resultList, 
					ListExpr.fourElemList(
							ListExpr.intAtom(rowIdx), 
							ListExpr.intAtom(slaveIdx), 
							ListExpr.boolAtom(succ), 
							taskResult.second()));
			cnt++;
		}
	
	}

	/* (non-Javadoc)
	 * @see org.apache.hadoop.mapreduce.Reducer#cleanup(org.apache.hadoop.mapreduce.Reducer.Context)
	 */
	@Override
	protected void cleanup(Context context)
			throws IOException, InterruptedException {

		context.write(new IntWritable(cnt), 
				new Text(resultList.toString().
						replaceAll("\n", " ").replaceAll("\t", " ")));
	}
}
