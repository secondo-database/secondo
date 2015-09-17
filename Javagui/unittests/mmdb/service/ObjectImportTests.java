package unittests.mmdb.service;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.fail;
import gui.SecondoObject;
import gui.idmanager.IDManager;

import java.io.File;
import java.io.IOException;

import mmdb.data.MemoryRelation;
import mmdb.data.attributes.MemoryAttribute;
import mmdb.data.attributes.standard.AttributeText;
import mmdb.error.inout.ImportException;
import mmdb.service.ObjectExport;
import mmdb.service.ObjectImport;

import org.junit.Test;

import unittests.mmdb.util.TestUtilRelation;

public class ObjectImportTests {

	@Test
	public void testImportObjectsAttribute() throws Exception {
		File temp = null;
		try {
			temp = File.createTempFile("MMSecondo_Test_Temp",
					ObjectExport.FILE_EXTENSION);
		} catch (IOException e) {
			fail("Could not create temp file! Cannot test MM-Import...");
		}
		MemoryAttribute attr = new AttributeText("AB\\nC");
		SecondoObject so = new SecondoObject(IDManager.getNextID());
		so.setName("EinString");
		so.setMemoryObject(attr);
		ObjectExport.getInstance().exportObjects(new SecondoObject[] { so },
				temp.getAbsolutePath());

		SecondoObject[] imported = ObjectImport.getInstance().importObjects(
				temp.getAbsolutePath());
		assertEquals(attr, imported[0].getMemoryObject());
		temp.delete();
	}

	@Test
	public void testImportObjectsMutliple() throws Exception {
		File temp = null;
		try {
			temp = File.createTempFile("MMSecondo_Test_Temp",
					ObjectExport.FILE_EXTENSION);
		} catch (IOException e) {
			fail("Could not create temp file! Cannot test MM-Import...");
		}
		MemoryRelation rel = TestUtilRelation.getIntStringRelation(5, false,
				false);
		SecondoObject so0 = new SecondoObject(IDManager.getNextID());
		so0.setName("Tabelle1");
		so0.setMemoryObject(rel);

		MemoryAttribute attr = new AttributeText("AB\\nC");
		SecondoObject so1 = new SecondoObject(IDManager.getNextID());
		so1.setName("EinString");
		so1.setMemoryObject(attr);

		ObjectExport.getInstance().exportObjects(
				new SecondoObject[] { so0, so1 }, temp.getAbsolutePath());

		SecondoObject[] imported = ObjectImport.getInstance().importObjects(
				temp.getAbsolutePath());
		
		assertEquals(so0.getMemoryObject(), imported[0].getMemoryObject());
		assertEquals(so1.getMemoryObject(), imported[1].getMemoryObject());
		temp.delete();
	}

	@Test(expected = ImportException.class)
	public void testImportObjectsFail1() throws Exception {
		ObjectImport.getInstance().importObjects("");
	}

	@Test(expected = ImportException.class)
	public void testImportObjectsFail2() throws Exception {
		File temp = null;
		try {
			temp = File.createTempFile("MMSecondo_Test_Temp", ".txt");
		} catch (IOException e) {
			fail("Could not create temp file! Cannot test MM-Import...");
		}
		try {
			ObjectImport.getInstance().importObjects(temp.getAbsolutePath());
		} finally {
			temp.delete();
		}
	}

}
