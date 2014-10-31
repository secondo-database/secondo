package com.secondo.webgui.server.rpc;
import java.io.ByteArrayOutputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.util.Iterator;
import java.util.List;

import javax.servlet.ServletException;
import javax.servlet.http.HttpServlet; 
import javax.servlet.http.HttpServletRequest; 
import javax.servlet.http.HttpServletResponse; 

import org.apache.commons.fileupload.FileItem;
import org.apache.commons.fileupload.FileItemFactory;
import org.apache.commons.fileupload.FileItemIterator; 
import org.apache.commons.fileupload.FileItemStream; 
import org.apache.commons.fileupload.FileUploadException;
import org.apache.commons.fileupload.disk.DiskFileItemFactory;
import org.apache.commons.fileupload.servlet.ServletFileUpload; 
import org.apache.commons.fileupload.servlet.ServletRequestContext;
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
        String fieldName = item.getFieldName();
        String fileName = item.getName();
        if (fileName != null) {
           fileName = FilenameUtils.getName(fileName);
        }
        String contentType = item.getContentType();
        boolean isInMemory = item.isInMemory();
        long sizeInBytes = item.getSize();
        System.out.print("Field Name:"+fieldName
        +",File Name:"+fileName);
        System.out.print("Content Type:"+contentType
        +",Is In Memory:"+isInMemory+",Size:"+sizeInBytes);			 
        byte[] data = item.get();
//        to specify path
        fileName = getServletContext()
                  .getRealPath( "/uploadedFiles/" + fileName);
        System.out.println("File name:" +fileName);			
        FileOutputStream fileOutSt = new FileOutputStream(fileName);
        fileOutSt.write(data);
        fileOutSt.close();
        System.out.println("File Uploaded Successfully!");
        response.getWriter().write("File name:"+fileName);
//        response.getWriter().write(new String(item.get())); 
		
//		 // Create a factory for disk-based file items
//		   FileItemFactory factory = new DiskFileItemFactory();
//		   // Create a new file upload handler
//		   ServletFileUpload upload = new ServletFileUpload(factory);
//		   try{
//		      // Parse the request
//		         List items = upload.parseRequest(request); 
//
//		         // Process the uploaded items
//		         Iterator iter = items.iterator();
//
//		         while (iter.hasNext()) {
//		            FileItem item = (FileItem) iter.next();
//		            //handling a normal form-field
//		            if(item.isFormField()) {
//		               System.out.println("Got a form field");
//		               String name = item.getFieldName();
//		               String value = item.getString();
//		               System.out.print("Name:"+name+",Value:"+value);				
//		            } else {//handling file loads
//		               System.out.println("Not form field");
//		               String fieldName = item.getFieldName();
//		               String fileName = item.getName();
//		               if (fileName != null) {
//		                  fileName = FilenameUtils.getName(fileName);
//		               }
//		               String contentType = item.getContentType();
//		               boolean isInMemory = item.isInMemory();
//		               long sizeInBytes = item.getSize();
//		               System.out.print("Field Name:"+fieldName
//		               +",File Name:"+fileName);
//		               System.out.print("Content Type:"+contentType
//		               +",Is In Memory:"+isInMemory+",Size:"+sizeInBytes);			 
//		               byte[] data = item.get();
//		               fileName = getServletContext()
//		                         .getRealPath( "/uploadedFiles/" + fileName);
//		               System.out.print("File name:" +fileName);			
//		               FileOutputStream fileOutSt = new FileOutputStream(fileName);
//		               fileOutSt.write(data);
//		               fileOutSt.close();
//		               System.out.print("File Uploaded Successfully!");
//		            }	
//		         }
//		    } catch(Exception e){
//		    	System.out.print("File Uploading Failed!" + e.getMessage());
//		}
    }
	
	 private FileItem getFileItem(HttpServletRequest request){
         FileItemFactory factory = new DiskFileItemFactory();
         // Set factory constraints
         //factory.setSizeThreshold(yourMaxMemorySize);
         //factory.setRepository(yourTempDirectory);

         ServletFileUpload upload = new ServletFileUpload(factory);
         //upload.setSizeMax(yourMaxRequestSize);

         try        {
                 List items = upload.parseRequest(request);
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
	
//	protected void service(final HttpServletRequest request,
//			HttpServletResponse response) throws ServletException, IOException {
//		 
//			boolean isMultiPart = ServletFileUpload
//					.isMultipartContent(new ServletRequestContext(request));
//		 
//			if(isMultiPart) {
//				FileItemFactory factory = new DiskFileItemFactory();
//				ServletFileUpload upload = new ServletFileUpload(factory);
//		 
//				try {
//					@SuppressWarnings("unchecked")
//					List items = upload.parseRequest(request);
//					FileItem uploadedFileItem = (FileItem) items.get(0); // we only upload one file
//		 
//					if(uploadedFileItem == null) {
//						super.service(request, response);
//						return;
//					} else if(uploadedFileItem.getFieldName().equalsIgnoreCase(
//							"uploadFormElement")) {
//						String fileName = uploadedFileItem.getName();
//						response.setStatus(HttpServletResponse.SC_CREATED);
//						response.getWriter().print("OK");
//						response.flushBuffer();
//					}
//		 
//				} catch(FileUploadException e) {
//					System.out.println(e);
//				}
//			}
//		 
//			else {
//				super.service(request, response);
//				return;
//			}
//		    }
    }
