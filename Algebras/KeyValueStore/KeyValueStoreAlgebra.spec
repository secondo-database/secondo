#This file is part of SECONDO.

#Copyright (C) 2004, University in Hagen, Department of Computer Science, 
#Database Systems for New Applications.

#SECONDO is free software; you can redistribute it and/or modify
#it under the terms of the GNU General Public License as published by
#the Free Software Foundation; either version 2 of the License, or
#(at your option) any later version.

#SECONDO is distributed in the hope that it will be useful,
#but WITHOUT ANY WARRANTY; without even the implied warranty of
#MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#GNU General Public License for more details.

#You should have received a copy of the GNU General Public License
#along with SECONDO; if not, write to the Free Software
#Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

#
#SERVERLIST
#
operator kvsAdd alias KVSADD pattern op (_, _, _)
operator kvsRemove alias KVSREMOVE pattern op (_)
operator kvsReconnect alias KVSRECONNECT pattern op (_)
operator kvsUpdateServerList alias KVSUPDATESERVERLIST pattern op (_)
operator kvsSyncServerList alias KVSSYNCSERVERLIST pattern op()
operator kvsSetDatabase alias KVSSETDATABASE pattern op (_)
operator kvsUseDatabase alias KVSUSEDATABASE pattern op (_)
operator kvsList alias KVSLIST pattern op()

#
#QUAD TREE
#
operator qtcreatedist alias QTCREATEDIST pattern _ op [_, _]
operator qtserveridLocal alias QTSERVERIDLOCAL pattern op (_, _)
operator qtserverid alias QTSERVERID pattern op (_, _)
operator qtintersectsLocal alias QTINTERSECTSLOCAL pattern op (_, _, _)
operator qtintersects alias QTINTERSECTS pattern op (_, _, _)
operator qtDistinct alias QTDISTINCT pattern op (_, _)

#
#DISTRIBUTION
#
operator kvsServerId alias KVSSERVERID pattern op (_, _, _)
operator kvsSaveDist alias KVSSAVEDIST pattern op (_)
operator kvsFilter alias KVSFILTER pattern _ op [_, _, _, _]

#
#KEY VALUE STORE
#
operator kvsStartApp alias KVSSTARTAPP pattern op()
operator kvsTransferId alias KVSTRANSFERID pattern op()
operator kvsGlobalId alias KVSGLOBALID pattern op()
operator kvsInitClients alias KVSINITCLIENTS pattern op(_, _, _)
operator kvsStartClient alias KVSSTARTCLIENT pattern op (_)
operator kvsStopClient alias KVSSTOPCLIENT pattern op (_)
operator kvsSetId alias KVSSETID pattern op (_)
operator kvsSetMaster alias KVSSETMASTER pattern op (_, _, _)
operator kvsRetrieve alias KVSRETRIEVE pattern _ op [_, _]
operator kvsExec alias KVSEXEC pattern op (_)

#
#NETWORKSTREAMS & DISTRIBUTION
#
operator kvsRemoteStream alias KVSREMOTESTREAM pattern op (_)
operator kvsRemoteStreamSCP alias KVSREMOTESTREAMSCP pattern op (_)
operator kvsDistribute alias KVSDISTRIBUTE pattern _ op [_, _, _, _, _, _]

#
#DATA SOURCE
#
operator kvsDataSourceSCP alias KVSDATASOURCESCP pattern op ()



# End KeyValueStoreAlgebra.spec