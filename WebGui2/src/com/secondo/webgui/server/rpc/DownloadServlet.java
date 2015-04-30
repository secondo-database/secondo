package com.secondo.webgui.server.rpc;

import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;
import javax.servlet.ServletException;
import javax.servlet.ServletOutputStream;
import javax.servlet.http.HttpServlet;
import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;

/**
 * This class represents a servlet to download text into a textfile.
 * 
 * @author Irina Russkaya
 * 
 **/
public class DownloadServlet extends HttpServlet {

	private static final long serialVersionUID = 1L;

	/**
	 * This method is called by the server (via the service method) to allow a
	 * servlet to handle a HTTP-GET request.
	 * 
	 * @param req
	 *            The HTTP Request object to provide client request information
	 *            to a servlet.
	 * @param response
	 *            The HTTP Response object to assist a servlet in sending a
	 *            response to the client
	 * */
	protected void doGet(HttpServletRequest req, HttpServletResponse response)
			throws ServletException, IOException {

		String fileName = req.getParameter("fileName");

		response.setContentType("application/octet-stream");
		response.setHeader("Content-Disposition",
				"attachment;filename=secondo-text.txt");

		File file = new File(fileName);
		FileInputStream fileIn = new FileInputStream(file);
		ServletOutputStream out = response.getOutputStream();

		byte[] outputByte = new byte[4096];
		// copy binary content to output stream
		while (fileIn.read(outputByte, 0, 4096) != -1) {
			out.write(outputByte, 0, 4096);
		}
		fileIn.close();
		out.flush();
		out.close();
	}
}
