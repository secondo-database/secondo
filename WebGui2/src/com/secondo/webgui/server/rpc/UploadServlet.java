package com.secondo.webgui.server.rpc;
import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.util.Iterator;
import java.util.List;

import javax.servlet.ServletContext;
import javax.servlet.ServletException;
import javax.servlet.http.HttpServlet; 
import javax.servlet.http.HttpServletRequest; 
import javax.servlet.http.HttpServletResponse; 

import org.apache.commons.fileupload.FileItem;
import org.apache.commons.fileupload.FileItemFactory;
import org.apache.commons.fileupload.FileUploadException;
import org.apache.commons.fileupload.disk.DiskFileItemFactory;
import org.apache.commons.fileupload.servlet.ServletFileUpload; 
import org.apache.commons.io.FilenameUtils;

public class UploadServlet extends HttpServlet{
    /**
	 * 
	 */
	private static final long serialVersionUID = -2057821650327615668L;

	protected void doPost(HttpServletRequest request, HttpServletResponse response)  throws ServletException, IOException {
		
		response.setContentType("text/plain");

        FileItem item = getFileItem(request);
        if (item == null) {
                response.getWriter().write("NO-SCRIPT-DATA");
                return;
        }
        
        String fileName = item.getName();
        if (fileName != null) {
           fileName = FilenameUtils.getName(fileName);
        }
        
        String contentType = item.getContentType();
        boolean isInMemory = item.isInMemory();
        long sizeInBytes = item.getSize();        
        System.out.print("Content Type:"+contentType
        +",Is In Memory:"+isInMemory+",Size:"+sizeInBytes);	
        
                
        ServletContext app = getServletContext();
        File tmpDir = (File)app.getAttribute("javax.servlet.context.tempdir");
        File targetFile = new File(tmpDir, fileName);        

        fileName=targetFile.getPath();
        byte[] data = item.get();
        System.out.println("File name to upload:" +fileName);			
        FileOutputStream fileOutSt = new FileOutputStream(fileName);
        fileOutSt.write(data);
        fileOutSt.close();
        System.out.println("File Uploaded Successfully!");
        response.getWriter().write("File name:"+fileName);

    }
	
	
	 private FileItem getFileItem(HttpServletRequest request){
         FileItemFactory factory = new DiskFileItemFactory();
         // Set factory constraints
         //factory.setSizeThreshold(yourMaxMemorySize);
         //factory.setRepository(yourTempDirectory);

         ServletFileUpload upload = new ServletFileUpload(factory);
         //upload.setSizeMax(yourMaxRequestSize);

         try        {
                 @SuppressWarnings("rawtypes")
				List items = upload.parseRequest(request);
                 @SuppressWarnings("rawtypes")
				Iterator it = items.iterator();
                 while (it.hasNext()) {
                         FileItem item = (FileItem) it.next();
                         if (!item.isFormField() && "upload".equals(item.getFieldName())){
                                 return item;
                         }
                 }
         } catch (FileUploadException e){
                 return null;
         }
         return null;
 } 
	

    }
