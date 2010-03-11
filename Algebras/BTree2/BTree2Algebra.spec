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

#operator createbtree alias CREATEBTREE pattern _ op [ _,_,_,_ ]
operator createbtree2 alias CREATEBTREE2 pattern _ op [ _,_,_,_ ]
operator insertbtree2 alias INSERTBTREE2 pattern _ _ op [ _, _]
operator insertbtree alias INSERTBTREE pattern _ _ op [ _ ]
operator deletebtree2 alias DELETEBTREE2 pattern _ _ op [ _, _]
operator deletebtree alias DELETEBTREE pattern _ _ op [ _ ]
operator updatebtree2 alias UPDATEBTREE2 pattern _ _ op [ _, _]
operator updatebtree alias UPDATEBTREE pattern _ _ op [ _ ]

operator exactmatch2 alias EXACTMATCH2 pattern _ op [ _ ]
operator exactmatchS alias EXACTMATCHS pattern _ op [ _ ]
operator exactmatch alias EXACTMATCH pattern _ _ op [ _ ]
operator range2 alias RANGE2 pattern _ op [ _, _ ]
operator rangeS alias RANGES pattern _ op [ _, _ ]
operator range alias RANGE pattern _ _ op [ _, _ ]
operator leftrange2 alias LEFTRANGE2 pattern _ op [ _ ]
operator leftrangeS alias LEFTRANGES pattern _ op [ _ ]
operator leftrange alias LEFTRANGE pattern _ _ op [ _ ]
operator rightrange2 alias RIGHTRANGE2 pattern _ op [ _ ]
operator rightrangeS alias RIGHTRANGES pattern _ op [ _ ]
operator rightrange alias RIGHTRANGE pattern _ _ op [ _ ]

operator keyrange2 alias KEYRANGE2 pattern _ op [ _ ]
operator keyrange alias KEYRANGE pattern _ _ op [ _ ]
operator getFileInfo alias GETFILEINFO pattern op ( _ )
operator treeheight alias TREEHEIGHT pattern op ( _ )
operator no_nodes alias NO_NODES pattern op ( _ )
operator no_entries alias NO_ENTRIES pattern op ( _ )
operator getRootNode alias GETROOTNODE pattern op ( _ )
operator getNodeInfo alias GETNODEINFO pattern op ( _ , _ )
operator getNodeSons alias GETNODESONS pattern op ( _ , _ )
operator internal_node_capacity alias INTERNAL_NODE_CAPACITY pattern op ( _ )
operator leaf_node_capacity alias LEAF_NODE_CAPACITY pattern op ( _ )
operator getMinFillDegree alias GETMINFILLDEGREE pattern op ( _ )
operator getNodeSize alias GETNODESIZE pattern op ( _ )

operator reset_counters alias RESET_COUNTERS pattern op ( _ )
operator set_cache_size alias SET_CACHE_SIZE pattern op ( _ , _ )
operator get_cache_size alias GET_CACHE_SIZE pattern op ( _ )
operator pin_nodes alias PIN_NODES pattern _ op [ _ ]
operator unpin_nodes alias UNPIN_NODES pattern _ op [ _ ]
operator get_pinned_nodes alias GET_PINNED_NODES pattern op ( _ )
operator get_no_nodes_visited alias GET_NO_NODES_VISITED pattern op ( _ )
operator get_no_cachehits alias GET_NO_CACHEHITS pattern op ( _ )
operator set_cache_limit_type alias SETCACHELIMITTYPE pattern op ( _ , _ )

operator set_maxkeysize alias SETMAXKEYSIZE pattern op(_)
operator get_maxkeysize alias GETMAXKEYSIZE pattern op(_)
operator set_maxvaluesize alias SETMAXVALUESIZE pattern op(_)
operator get_maxvaluesize alias GETMAXVALUESIZE pattern op(_)
operator get_statistics alias GETSTATISTICS pattern op(_)
operator set_debug alias SETDEBUG pattern op(_)

operator getentry2 alias GETENTRY2 pattern op ( _ )
