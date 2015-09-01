package mmdb.streamprocessing.parser.nestedlist;

import java.util.ArrayList;
import java.util.Collections;
import java.util.List;

public class ListNode extends NestedListNode {

	List<NestedListNode> children = new ArrayList<NestedListNode>();

	public ListNode(ArrayList<NestedListNode> children) {
		this.children = children;
	}

	public List<NestedListNode> getChildren() {
		return Collections.unmodifiableList(children);
	}

	@Override
	public String printValueList() {
		String retVal = "(";
		for(int i = 0; i < children.size(); i++) {
			retVal += children.get(i).printValueList();
			if(i < children.size() -1) {
				retVal += " ";
			}
		}
		retVal += ")";
		return retVal;
	}
	
	@Override
	public String printTypeList() {
		String retVal = "(";
		for(int i = 0; i < children.size(); i++) {
			retVal += children.get(i).printTypeList();
			if(i < children.size() -1) {
				retVal += " ";
			}
		}
		retVal += ")";
		return retVal;
	}

}
