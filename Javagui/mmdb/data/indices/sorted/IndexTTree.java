//This file is part of SECONDO.

//Copyright (C) 2014, University in Hagen, Department of Computer Science,
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

package mmdb.data.indices.sorted;

import java.util.ArrayList;
import java.util.Collections;
import java.util.List;

import mmdb.data.MemoryTuple;
import mmdb.data.features.Orderable;
import mmdb.error.index.IndexAddElementException;
import mmdb.error.index.IndexSearchElementException;

/**
 * This class provides a T-Tree index structure.
 *
 * @author Alexander Castor
 */
public class IndexTTree extends SortedIndex {

	/**
	 * The maximum capacity of the nodes' data arrays.
	 */
	private static final int CAPACITY = 1;

	/**
	 * The tree's root element.
	 */
	private TNode root;

	/*
	 * (non-Javadoc)
	 * 
	 * @see mmdb.data.indices.MemoryIndex#insertElement(java.lang.Object,
	 * mmdb.data.MemoryTuple)
	 */
	@Override
	protected void insertElement(Orderable attribute, MemoryTuple tuple)
			throws IndexAddElementException {
		try {
			TreeElement element = new TreeElement(attribute, tuple);
			if (root == null) {
				TNode node = new TNode(element, null, null);
				root = node;
				return;
			}
			TNode node = search(element);
			element = insertElement(node, element);
			if (element != null) {
				root = insertLeaf(root, element);
			}
		} catch (Throwable e) {
			e.printStackTrace();
			throw new IndexAddElementException(
					"-> Technical error when inserting element to t-tree index.");
		}
	}

	/*
	 * (non-Javadoc)
	 * 
	 * @see mmdb.data.indices.MemoryIndex#searchElements(java.lang.Object)
	 */
	@Override
	public List<MemoryTuple> searchElements(Orderable attribute) throws IndexSearchElementException {
		try {
			if (root == null) {
				return new ArrayList<MemoryTuple>();
			}
			TreeElement element = new TreeElement(attribute, null);
			TNode node = search(element);
			int index = node.elements.indexOf(element);
			if (index < 0) {
				return new ArrayList<MemoryTuple>();
			} else {
				return node.elements.get(index).tuples;
			}
		} catch (Throwable e) {
			throw new IndexSearchElementException(
					"-> Technical error when searching element in t-tree index.");
		}
	}

	/**
	 * Searches for the node that must contain the given element.
	 * 
	 * @param element
	 *            the element to be searched
	 * @return the node which must contain the given element
	 */
	private TNode search(TreeElement element) {
		TNode node = root;
		while (true) {
			boolean smaller = element.compareTo(node.elements.getMin()) < 0;
			boolean bigger = element.compareTo(node.elements.getMax()) > 0;
			boolean leftHalfLeaf = node.leftChild == null;
			boolean rightHalfLeaf = node.rightChild == null;
			boolean found = (!smaller && !bigger) || (leftHalfLeaf && smaller)
					|| (rightHalfLeaf && bigger);
			if (found) {
				break;
			}
			if (smaller) {
				node = node.leftChild;
			} else {
				node = node.rightChild;
			}
		}
		return node;
	}

	/**
	 * Tries to insert the given element in one of the leaves.
	 * 
	 * @param node
	 *            the element's bounding node
	 * @param element
	 *            the element to be inserted
	 * @return null, if the element could be inserted without a new leaf, else
	 *         the element to be inserted
	 */
	private TreeElement insertElement(TNode node, TreeElement element) {
		while (true) {
			if (node == null) {
				return element;
			}
			int index = node.elements.indexOf(element);
			if (index >= 0) {
				node.elements.get(index).tuples.addAll(element.tuples);
				return null;
			}
			boolean hasCapacity = node.elements.size() < CAPACITY;
			if (hasCapacity) {
				node.elements.add(element);
				return null;
			}
			TreeElement tmp = node.elements.getMin();
			node.elements.remove(tmp);
			node.elements.add(element);
			element = tmp;
			node = node.leftChild;
		}
	}

	/**
	 * Inserts a new element leaf the tree containing the given element and
	 * performs a rotation if necessary.
	 * 
	 * @param node
	 *            the current node
	 * @param element
	 *            the element to be inserted
	 * @return the root of the next subtree
	 */
	private TNode insertLeaf(TNode node, TreeElement element) {
		if (node == null) {
			node = new TNode(element, null, null);
		} else {
			if (element.compareTo(node.elements.getMin()) < 0) {
				node.leftChild = insertLeaf(node.leftChild, element);
				if (getHeight(node.leftChild) - getHeight(node.rightChild) == 2) {
					if (element.compareTo(node.leftChild.elements.getMin()) < 0) {
						node = rotateOnceLeft(node);
					} else {
						node = rotateDoubleLeft(node);
					}
				}
			} else {
				node.rightChild = insertLeaf(node.rightChild, element);
				if (getHeight(node.rightChild) - getHeight(node.leftChild) == 2) {
					if (element.compareTo(node.rightChild.elements.getMin()) > 0) {
						node = rotateOnceRight(node);
					} else {
						node = rotateDoubleRight(node);
					}
				}
			}
		}
		node.height = Math.max(getHeight(node.leftChild), getHeight(node.rightChild)) + 1;
		return node;
	}

	/**
	 * Performs a single rotation with the given node's left child.
	 * 
	 * @param node
	 *            the node to be rotated
	 * @return the rotated node
	 */
	private TNode rotateOnceLeft(TNode node) {
		TNode result = node.leftChild;
		node.leftChild = result.rightChild;
		result.rightChild = node;
		node.height = Math.max(getHeight(node.leftChild), getHeight(node.rightChild)) + 1;
		result.height = Math.max(getHeight(result.leftChild), getHeight(node)) + 1;
		return result;
	}

	/**
	 * Performs a double rotation with the given node's left child.
	 * 
	 * @param node
	 *            the node to be rotated
	 * @return the rotated node
	 */
	private TNode rotateDoubleLeft(TNode node) {
		node.leftChild = rotateOnceRight(node.leftChild);
		return rotateOnceLeft(node);
	}

	/**
	 * Performs a single rotation with the given node's right child.
	 * 
	 * @param node
	 *            the node to be rotated
	 * @return the rotated node
	 */
	private TNode rotateOnceRight(TNode node) {
		TNode result = node.rightChild;
		node.rightChild = result.leftChild;
		result.leftChild = node;
		node.height = Math.max(getHeight(node.leftChild), getHeight(node.rightChild)) + 1;
		result.height = Math.max(getHeight(result.rightChild), getHeight(node)) + 1;
		return result;
	}

	/**
	 * Performs a double rotation with the given node's right child.
	 * 
	 * @param node
	 *            the node to be rotated
	 * @return the rotated node
	 */
	private TNode rotateDoubleRight(TNode node) {
		node.rightChild = rotateOnceLeft(node.rightChild);
		return rotateOnceRight(node);
	}

	/**
	 * Retrieves the height for a given node.
	 * 
	 * @param node
	 *            the node whose height shall be retrieved
	 * @return the node's height or -1 if node is null
	 */
	private int getHeight(TNode node) {
		if (node == null) {
			return -1;
		} else {
			return node.height;
		}
	}

	/**
	 * A node of the tree containing references to both children and the data
	 * array.
	 *
	 * @author Alexander Castor
	 */
	private static class TNode {

		int height = 0;
		TNode leftChild;
		TNode rightChild;
		SortedList<TreeElement> elements = new SortedList<TreeElement>();

		TNode(TreeElement element, TNode leftChild, TNode rightChild) {
			elements.add(element);
			this.leftChild = leftChild;
			this.rightChild = rightChild;
		}

	}

	/**
	 * A tree element consisting of the attribute to be used for indexing and
	 * references to the corresponding tuples.
	 *
	 * @author Alexander Castor
	 */
	private static class TreeElement implements Comparable<TreeElement> {

		Orderable attribute;
		List<MemoryTuple> tuples;

		TreeElement(Orderable attribute, MemoryTuple tuple) {
			this.attribute = attribute;
			tuples = new ArrayList<MemoryTuple>();
			tuples.add(tuple);
		}

		@Override
		public int compareTo(TreeElement o) {
			return attribute.compareTo(o.attribute);
		}

		@Override
		public boolean equals(Object o) {
			if (o == null) {
				return false;
			}
			TreeElement other = (TreeElement) o;
			return attribute.equals(other.attribute);
		}

	}

	/**
	 * ArrayList that maintains the order of elements.
	 *
	 * @author Alexander Castor
	 */
	@SuppressWarnings({ "serial" })
	private static class SortedList<T extends Comparable<T>> extends ArrayList<T> {

		@Override
		public boolean add(T element) {
			int index = Math.abs(Collections.binarySearch(this, element) + 1);
			super.add(index, element);
			return true;
		}

		SortedList() {
			super(CAPACITY);
		}

		T getMin() {
			return this.get(0);
		}

		T getMax() {
			return this.get(this.size() - 1);
		}

	}

}
