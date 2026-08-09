//This file is part of SECONDO.

//Copyright (C) 2004, University in Hagen, Department of Computer Science,
//Database Systems for New Applications.

//SECONDO is free software; you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation; either version 2 of the License, or
//(at your option) any later version.

//SECONDO is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

//You should have received a copy of the GNU General Public License
//along with SECONDO; if not, write to the Free Software
//Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

package sj.lang;

import java.io.IOException;
import java.io.InputStream;

/**
 * The reading half of a <SecondoResult> frame: pulls records off the socket
 * and serves the D bodies as one continuous byte stream, so a list reader sees
 * the result and never the framing.
 *
 * The frame is described in include/CSProtocol.h, which is the protocol's
 * specification; the C++ counterpart of this class is csp::ChunkedInBuf there.
 * In short: each record is one tag byte, decimal fields and a newline followed
 * by exactly as many body bytes as the header announced. "D" carries a piece of
 * the result, "M" a message raised while the result was being written, and "E"
 * or "A" ends the frame and carries the command's status -- "A" meaning the
 * server stopped early and the bytes so far are an incomplete encoding.
 *
 * Reading incrementally is the point. Collecting the whole payload before
 * parsing any of it would cost 165 MB for a query like "query roads", which is
 * a straight regression against reading the list off the socket as it arrives.
 *
 * Because the records are length-delimited, recovering from a payload the list
 * reader could not make sense of does not require understanding the payload:
 * drainToTerminator() skips records until the frame ends, and the connection is
 * then positioned at the next response.
 */
public class ChunkedInputStream extends InputStream {

/**
 * Where an M record goes. The frame reader has the bytes but neither the
 * transfer mode nor the listeners, so it hands the body on rather than
 * interpreting it.
 */
   public interface MessageSink {
      void frameMessage(byte[] body);
   }

   private static final String END_RESULT = "</SecondoResult>";

   private final MyDataInputStream in;
   private final MessageSink sink;

   // the current D body, and how much of it has been served
   private byte[] buffer = new byte[0];
   private int pos = 0;
   private int limit = 0;

   private boolean finished = false;
   private boolean broken = false;
   private char kind = 0;
   private int frameErrorCode = 0;
   private int frameErrorPos = 0;
   private String frameMessage = "";

   public ChunkedInputStream(MyDataInputStream in, MessageSink sink) {
      this.in = in;
      this.sink = sink;
   }

   public int read() throws IOException {
      if (!ensure()) {
         return -1;
      }
      return buffer[pos++] & 0xff;
   }

   public int read(byte[] b, int off, int len) throws IOException {
      if (len == 0) {
         return 0;
      }
      if (!ensure()) {
         return -1;
      }
      int n = Math.min(len, limit - pos);
      System.arraycopy(buffer, pos, b, off, n);
      pos += n;
      return n;
   }

   public int available() {
      return limit - pos;
   }

/**
 * Skip records until the frame's terminator has been consumed. Safe at any
 * point, including after a parse that gave up half way: the framing is
 * length-delimited, so nothing here has to understand the bytes it discards.
 */
   public void drainToTerminator() throws IOException {
      pos = limit;
      while (!finished && !broken) {
         readRecord();
         pos = limit;
      }
   }

/** true unless the framing itself was lost */
   public boolean ok() {
      return !broken;
   }

/**
 * The terminator, valid once the stream has ended: 'E' if the payload is a
 * complete encoding, 'A' if the server aborted mid-result, 0 if the frame
 * broke.
 */
   public char kind() {
      return kind;
   }

   public int errorCode() {
      return frameErrorCode;
   }

   public int errorPos() {
      return frameErrorPos;
   }

   public String message() {
      return frameMessage;
   }

/** refill the get area, pulling records until one carries data */
   private boolean ensure() throws IOException {
      while (pos >= limit) {
         if (finished || broken) {
            return false;
         }
         readRecord();
      }
      return true;
   }

   private void readRecord() throws IOException {
      int tag = in.read();
      if (tag < 0) {
         fail();
         return;
      }
      String header = readHeaderLine();
      if (header == null) {
         fail();
         return;
      }
      String[] fields = header.trim().split("\\s+");

      if (tag == 'D' || tag == 'M') {
         if (fields.length != 1) {
            fail();
            return;
         }
         byte[] body = readBody(fields[0]);
         if (body == null) {
            return;
         }
         if (tag == 'D') {
            buffer = body;
            pos = 0;
            limit = body.length;
         } else if (sink != null) {
            sink.frameMessage(body);
         }
         return;
      }

      if (tag == 'E' || tag == 'A') {
         if (fields.length != 3) {
            fail();
            return;
         }
         int code;
         int errPos;
         try {
            code = Integer.parseInt(fields[0]);
            errPos = Integer.parseInt(fields[1]);
         } catch (NumberFormatException e) {
            fail();
            return;
         }
         byte[] body = readBody(fields[2]);
         if (body == null) {
            return;
         }
         kind = (char) tag;
         frameErrorCode = code;
         frameErrorPos = errPos;
         frameMessage = new String(body);
         finished = true;
         // The frame is over; the tag closing it is an ordinary line again.
         String line = readHeaderLine();
         if (line == null || !line.equals(END_RESULT)) {
            broken = true;
         }
         return;
      }

      // Anything else means the framing is lost, and there is no way to find
      // the next record boundary once that has happened.
      fail();
   }

   private byte[] readBody(String lengthField) throws IOException {
      int n;
      try {
         n = Integer.parseInt(lengthField);
      } catch (NumberFormatException e) {
         fail();
         return null;
      }
      if (n < 0) {
         fail();
         return null;
      }
      byte[] body = new byte[n];
      int got = 0;
      while (got < n) {
         int r = in.read(body, got, n - got);
         if (r < 0) {
            fail();
            return null;
         }
         got += r;
      }
      return body;
   }

/**
 * Read to the next newline, byte by byte off the socket.
 *
 * Deliberately not a BufferedReader: a reader would buffer past the newline
 * into the record body that follows it, and the body is raw bytes that only
 * this class knows the length of.
 */
   private String readHeaderLine() throws IOException {
      StringBuilder sb = new StringBuilder(32);
      while (true) {
         int c = in.read();
         if (c < 0) {
            return null;
         }
         if (c == '\n') {
            return sb.toString();
         }
         sb.append((char) c);
      }
   }

   private void fail() {
      broken = true;
      finished = true;
      kind = 0;
      pos = 0;
      limit = 0;
   }
}
