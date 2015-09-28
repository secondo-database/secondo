package unittests.mmdb.service;

import static org.junit.Assert.fail;
import gui.SecondoObject;
import gui.idmanager.IDManager;

import java.io.File;
import java.io.IOException;

import mmdb.data.MemoryRelation;
import mmdb.data.MemoryTuple;
import mmdb.data.attributes.MemoryAttribute;
import mmdb.data.attributes.standard.AttributeText;
import mmdb.error.inout.ExportException;
import mmdb.service.ObjectExport;

import org.junit.Test;

import unittests.mmdb.util.TestUtilMocks.ImportExportGuiMock;
import unittests.mmdb.util.TestUtilRelation;

public class ObjectExportTests {

	@Test
	public void testExportObjectsRelation() throws Exception {
		File temp = null;
		try {
			temp = File.createTempFile("MMSecondo_Test_Temp",
					ObjectExport.FILE_EXTENSION);
		} catch (IOException e) {
			fail("Could not create temp file! Cannot test MM-Export...");
		}
		MemoryRelation rel = TestUtilRelation.getIntStringRelation(5, false,
				false);
		SecondoObject so = new SecondoObject(IDManager.getNextID());
		so.setName("Tabelle1");
		so.setMemoryObject(rel);
		ObjectExport objectExport = new ObjectExport(
				new SecondoObject[] { so }, temp.getAbsolutePath(),
				new ImportExportGuiMock());
		objectExport.doInBackground();
		temp.delete();
	}

	@Test
	public void testExportObjectsAttribute() throws Exception {
		File temp = null;
		try {
			temp = File.createTempFile("MMSecondo_Test_Temp",
					ObjectExport.FILE_EXTENSION);
		} catch (IOException e) {
			fail("Could not create temp file! Cannot test MM-Export...");
		}
		MemoryAttribute attr = new AttributeText("AB\\nC");
		SecondoObject so = new SecondoObject(IDManager.getNextID());
		so.setName("EinString");
		so.setMemoryObject(attr);
		ObjectExport objectExport = new ObjectExport(
				new SecondoObject[] { so }, temp.getAbsolutePath(),
				new ImportExportGuiMock());
		objectExport.doInBackground();
		temp.delete();
	}

	@Test
	public void testExportObjectsMultiple() throws Exception {
		File temp = null;
		try {
			temp = File.createTempFile("MMSecondo_Test_Temp",
					ObjectExport.FILE_EXTENSION);
		} catch (IOException e) {
			fail("Could not create temp file! Cannot test MM-Export...");
		}
		MemoryRelation rel = TestUtilRelation.getIntStringRelation(5, false,
				false);
		SecondoObject so1 = new SecondoObject(IDManager.getNextID());
		so1.setName("Tabelle1 [+]");
		so1.setMemoryObject(rel);

		MemoryAttribute attr = new AttributeText("AB\\nC");
		SecondoObject so2 = new SecondoObject(IDManager.getNextID());
		so2.setName("EinString [++]");
		so2.setMemoryObject(attr);

		ObjectExport objectExport = new ObjectExport(new SecondoObject[] { so1,
				so2 }, temp.getAbsolutePath(), new ImportExportGuiMock());
		objectExport.doInBackground();
		temp.delete();
	}

	@Test(expected = ExportException.class)
	public void testExportObjectsFail1() throws Exception {
		File temp = null;
		try {
			temp = File.createTempFile("MMSecondo_Test_Temp",
					ObjectExport.FILE_EXTENSION);
		} catch (IOException e) {
			fail("Could not create temp file! Cannot test MM-Export...");
		}
		try {
			SecondoObject so1 = new SecondoObject(IDManager.getNextID());
			so1.setName("Tabelle1");

			ObjectExport.checkObjectsExportable(new SecondoObject[] { so1 });
		} finally {
			temp.delete();
		}
	}

	@Test(expected = ExportException.class)
	public void testExportObjectsFail2() throws Exception {
		File temp = null;
		try {
			temp = File.createTempFile("MMSecondo_Test_Temp",
					ObjectExport.FILE_EXTENSION);
		} catch (IOException e) {
			fail("Could not create temp file! Cannot test MM-Export...");
		}
		try {
			SecondoObject so1 = new SecondoObject(IDManager.getNextID());
			so1.setName("Tabelle1");
			so1.setMemoryObject(new MemoryTuple());

			ObjectExport.checkObjectsExportable(new SecondoObject[] { so1 });
		} finally {
			temp.delete();
		}
	}

}
