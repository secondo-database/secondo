package util;


/*
 * https://github.com/SpringSource/spring-hadoop/blob/master/src/main/java/org/springframework/data/hadoop/config/LinkedProperties.java
 */



import java.util.Collections;
import java.util.Enumeration;
import java.util.LinkedHashSet;
import java.util.Properties;
import java.util.Set;

class LinkedProperties extends Properties {

private static final long serialVersionUID = 1L;
private final LinkedHashSet<Object> keys = new LinkedHashSet<Object>();
/**
	 * 
	 */
	public LinkedProperties() 
	{
		super();
	}

public Enumeration<Object> keys() 
{
	return Collections.<Object> enumeration(keys);
}

public Object put(Object key, Object value) 
{
	keys.add(key);
	return super.put(key, value);
}

	public Set<String> stringPropertyNames() 
	{
		Set<String> set = new LinkedHashSet<String>();
	
		for (Object key : this.keys) 
		{
		if (key instanceof String && this.get(key) instanceof String)
		set.add((String) key);
		}
	
		return set;
	}

	public Set<Object> keySet() 
	{
		return keys;
	}

}
