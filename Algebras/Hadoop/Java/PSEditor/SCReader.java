package PSEditor;

import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.File;
import java.io.FileReader;
import java.io.FileWriter;
import java.io.IOException;
import java.util.ArrayList;
import java.util.Arrays;


/**
 * This is a class prepared to read the SECONDO configuration file.
 * A parameter is composed by title and value,
 * Two kinds of parameters are used in this file, defined by the enumeration MODE
 *   - SINGLE: A parameter has a title and a value. 
 *   - MULTI:	A parameter has multiple values for one title, divided by spaces. 
 * 
 * This class can also read the ParallelSecondoConfig.ini  
 *  

* 
*/

public class SCReader {
	
	public static enum MODE{SINGLE, MULTI};
	
	private String filePath;	//Path of the configuration file
	private ArrayList<Section> sections;
	private BufferedReader reader;
	
	private final String multiMark = " ";
	
	public static void main(String[] args)
	{
		String inPath = args[0];
		String outPath = args[1];
		
		try {
			SCReader reader = new SCReader(inPath);
			reader.list(outPath);
		} catch (IOException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
	}
	
	public SCReader(String path) throws IOException
	{
		filePath = path;
		sections = new ArrayList<Section>();
		if (new File(path).exists())
			load();
	}
	
	public boolean isEmpty()
	{
		return sections.isEmpty();
	}
	
	public SCReader()
	{
		filePath = "";
		sections = new ArrayList<Section>();
	}
	
	private void load() throws IOException 
	{
		reader = new BufferedReader(new FileReader(filePath));
		//Load the parameters from the configuration file.
		while (reader.ready())
		{
			String line = reader.readLine().trim();
			
			if (line.isEmpty() || line.startsWith("#"))
				continue;
			else if (line.startsWith("[") && line.endsWith("]"))
			{
				String secName = line.substring(1,line.length() - 1).trim();
				sections.add(new Section(secName));
				while (reader.ready())
				{
					reader.mark(0);
					line = reader.readLine().trim();
					if (line.isEmpty() || line.startsWith("#"))
						continue;
					else if (line.startsWith("[") && line.endsWith("]"))
					{
						reader.reset();
						break;
					}
					else
					{
						//Read the line belonging to the current section. 
						if (!line.contains("+=") && line.split("=").length == 2)
						{
							//MODE = SINGLE
							String[] kv = line.split("=");
							set(secName, kv[0].trim(), kv[1].trim(), MODE.SINGLE);
						}
						else if (line.split("[+]=").length == 2)
						{
							//MODE = MULTI
							String[] kv = line.split("[+]=");
							set(secName, kv[0].trim(), kv[1].trim(), MODE.MULTI);
						}
						else
						{
							String errInfo = "Invalid parameter";
							System.err.println(errInfo);
							throw new IOException(errInfo);
						}
					}
				}
			}
		}
		reader.close();
	}
	
	//Get the value with section s and title t
	public String get(String s, String t)
	{
		//Serach only one section
	  for (Section sec : sections)
	  {
			if (sec.getSectionName().compareTo(s) == 0)
			{
				return sec.get(t);				
		 	}
		}
		return "";
	}
	
	public void delSection(String s)
	{
		for (Section sec : sections)
		{
			if (sec.getSectionName().compareTo(s) == 0)
			{
				sections.remove(sec);
				return;
			}
		}
	}
	
	public void delSinglePara(String s, String t)
	{
		for (Section sec : sections)
		{
			if (sec.getSectionName().compareTo(s) == 0)
			{
				sec.delSinglePara(t);
				return;
			}
		}
	}
	
	public void delMultiPara(String s, String t, String v)
	{
		for (Section sec : sections)
		{
			if (sec.getSectionName().compareTo(s) == 0)
			{
				sec.delMultiPara(t, v);
			}
		}
	}
	
	
	public ArrayList<String> getTitles(String s)
	{
		for (Section sec : sections)
		{
			if (sec.getSectionName().compareTo(s) == 0)
			{
				return sec.getTitles();
			}
		}
		return new ArrayList<String>();
	}
	
	public ArrayList<String> getSectionNames()
	{
		ArrayList<String> secNames = new ArrayList<String>();
		for (Section sec : sections)
			secNames.add(sec.getSectionName());
		
		return secNames;
	}
	
	/**
	 * Export all parameters to a file
	 * 
	 * @param path
	 */
	public void list(String path) throws IOException
	{
		BufferedWriter writer = new BufferedWriter(new FileWriter(path));
		
		for (Section sec : sections)
		{
			if (sec.getSectionName().compareTo("ParallelSecondo") == 0){
				//Ignore the ParallelSecondo relative parameters, 
				//which will be added by the install script
				continue;
			}
			
			writer.write("[" + sec.getSectionName() + "]");
			writer.newLine();
			ArrayList<String> titles = sec.getTitles();
			for (String t : titles)
			{
				String v = sec.get(t);
				boolean multiMode = v.contains(multiMark);
				
				if (!multiMode)
				{
					//It is a SINGLE mode parameter
					writer.write(t + " = " + v);
					writer.newLine();
				}
				else
				{
					//It is a MULTI mode parameter
					String[] values = v.trim().split(multiMark);
					for (String sv : values)
					{
						writer.write(t + " += " + sv);
						writer.newLine();
					}
				}
				
			}
		}
		writer.flush();
		writer.close();
	}
	
	/**
	 * 
	 * @param s: section
	 * @param t: title
	 * @param v: value
	 * @param m: mode
	 * 
	 * Set one parameter, if the parameter is already set, 
	 * it is added (+=) with the new value if the mode is MULTI,
	 * or else its value is replaced by the new value.   
	 * @throws IOException 
	 * 
	 */
	public void set(String s, String t, String v, MODE m)
	{
		boolean found = false;
		for (Section sec : sections)
		{
			if (sec.getSectionName().compareTo(s) == 0)
			{
				found = true;
				sec.set(t, v, m);
				break;
			}
		}
		
		if (!found)
		{
			sections.add(new Section(s, t, v, m));
		}
	}
	
	public MODE getType(String s, String t)
	{
		String value = "";
		//Serach only one section
	  for (Section sec : sections)
	  {
			if (sec.getSectionName().compareTo(s) == 0)
			{
				value = sec.get(t);
				break;
		 	}
		}
		
	  if (value.contains(multiMark))
	  	return MODE.MULTI;
	  else
	  	return MODE.SINGLE;
	}
	
	public ArrayList<String> getMultiValues(String s, String t)
	{
		if (getType(s,t) == MODE.MULTI)
		{
			return new ArrayList<String>(
					Arrays.asList(get(s,t).split(multiMark)));
		}
		return new ArrayList<String>();
	}

class Section {
	private String sectionName;
	private ArrayList<Parameter> parameters; 
	
	Section(String s)
	{
		sectionName = s;
		parameters = new ArrayList<Parameter>();
	}
	
	Section(String s, String t, String v, MODE m)
	{
		sectionName = s;
		parameters = new ArrayList<Parameter>();
		parameters.add(new Parameter(t, v, m));
	}
	
	public void set(String t, String v, SCReader.MODE m)
	{
		boolean found = false;
		for (Parameter p : parameters)
		{
			if (p.getTitle().compareTo(t) == 0)
			{
				found = true;
				p.set(v, m);
				break;
			}
		}
		
		if (!found)
		{
			parameters.add(new Parameter(t, v, m));
		}
	}
	
	public String get(String t)
	{
		for (Parameter p : parameters)
		{
			if (p.getTitle().compareTo(t) == 0)
			{
				return p.getValue();
			}
		}
		return "";
	}
	
	public String getSectionName()
	{
		return sectionName;
	}
	
	public ArrayList<String> getTitles()
	{
		ArrayList<String> titles = new ArrayList<String>();
		for (Parameter p : parameters)
		{
			titles.add(p.getTitle());
		}
		return titles;
	}
	
	public void delSinglePara(String t)
	{
		for (Parameter p : parameters)
		{
			if (p.getTitle().compareTo(t) == 0){
				parameters.remove(p);
				return;
			}
		}
	}
	
	public void delMultiPara(String t, String v)
	{
		for (Parameter p : parameters)
		{
			if (p.getTitle().compareTo(t) == 0){
				String newValue = p.getValue().replaceAll(v + multiMark, "");
				if (newValue.isEmpty())
					parameters.remove(p);
				else
					p.set(newValue, MODE.SINGLE); //set all values for once 
			}
		}
	}
}

class Parameter{
	private String title;
	private String value;
	
	Parameter(String t, String v, MODE m)
	{
		title = t;
		if (m == MODE.SINGLE)
			value = v;
		else 
			value = v + multiMark;
	}
	
	public void set(String v, SCReader.MODE m)
	{
		if (m == SCReader.MODE.SINGLE){
			value = v;
		}
		else{
			value += (v + multiMark);
		}
	}
	
	public String getValue()
	{
		return value;
	}
	
	public String getTitle()
	{
		return title;
	}
}

}

