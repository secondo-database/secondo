package unittests.mmdb.util;

import gui.SecondoObject;
import gui.idmanager.IDManager;

import java.util.ArrayList;
import java.util.List;

import mmdb.data.MemoryObject;

public class TestUtilParser {

	public static List<SecondoObject> getList(SecondoObject... sobjects) {
		ArrayList<SecondoObject> result = new ArrayList<SecondoObject>();
		for (SecondoObject sobject : sobjects) {
			result.add(sobject);
		}
		return result;
	}

	public static SecondoObject getSecondoObject(MemoryObject mobject,
			String name) {
		SecondoObject sobject = new SecondoObject(IDManager.getNextID());
		sobject.setName(name);
		sobject.setMemoryObject(mobject);
		return sobject;
	}

}
