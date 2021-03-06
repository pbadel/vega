/*
 * Copyright (C) Alneos, s. a r. l. (contact@alneos.fr) 
 * Unauthorized copying of this file, via any medium is strictly prohibited. 
 * Proprietary and confidential.
 *
 * MeshComponents.h
 *
 *  Created on: Nov 4, 2013
 *      Author: devel
 */

#ifndef MESHCOMPONENTS_H_
#define MESHCOMPONENTS_H_

#include "CoordinateSystem.h"
#include "Dof.h"
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <string>
#include <stdexcept>
#include <iterator>
#ifdef __GNUC__
// Avoid tons of warnings with root code
#pragma GCC system_header
#endif
#define MESGERR 1
#include <med.h>

namespace vega {

class SpaceDimension final {

public:

	enum Code {
		DIMENSION0D_CODE = 0,
		DIMENSION1D_CODE = 1,
		DIMENSION2D_CODE = 2,
		DIMENSION3D_CODE = 3
	};

	// Enum value DECLARATIONS - they are defined later
	static std::unordered_map<SpaceDimension::Code, SpaceDimension*, std::hash<int>> dimensionByCode;
	static const SpaceDimension DIMENSION_0D;
	static const SpaceDimension DIMENSION_1D;
	static const SpaceDimension DIMENSION_2D;
	static const SpaceDimension DIMENSION_3D;

private:

	SpaceDimension(Code code, int medcouplingRelativeMeshDimension);

	static std::unordered_map<SpaceDimension::Code, SpaceDimension*, std::hash<int>> init_map() {
		return std::unordered_map<SpaceDimension::Code, SpaceDimension*, std::hash<int>>();
	}

public:
	Code code;
	int relativeMeshDimension;
	bool operator<(const SpaceDimension &other) const;
	inline bool operator>(const SpaceDimension &other) const {
		return this->code > other.code;
	}
	bool operator==(const SpaceDimension &other) const;
};

class CellType final {

public:

	enum Code {
		//codes used here are the same in Med
		POINT1_CODE = MED_POINT1,
		SEG2_CODE = MED_SEG2,
		SEG3_CODE = MED_SEG3,
		SEG4_CODE = MED_SEG4,
		POLYL_CODE = MED_POLYGON,
		TRI3_CODE = MED_TRIA3,
		QUAD4_CODE = MED_QUAD4,
		POLYGON_CODE = MED_POLYGON,
		TRI6_CODE = MED_TRIA6,
		TRI7_CODE = MED_TRIA7,
		QUAD8_CODE = MED_QUAD8,
		QUAD9_CODE = MED_QUAD9,
		//QPOLYG_CODE = MED_POLYGON2,
		//
		TETRA4_CODE = MED_TETRA4,
		PYRA5_CODE = MED_PYRA5,
		PENTA6_CODE = MED_PENTA6,
		HEXA8_CODE = MED_HEXA8,
		TETRA10_CODE = MED_TETRA10,
		HEXGP12_CODE = MED_OCTA12,
		PYRA13_CODE = MED_PYRA13,
		PENTA15_CODE = MED_PENTA15,
		HEXA20_CODE = MED_HEXA20,
		HEXA27_CODE = MED_HEXA27,
		POLYHED_CODE = MED_POLYHEDRON,
		//type for reserved (but not yet defined) scells
		RESERVED = -1
	};

	// Enum value DECLARATIONS - they are defined later
	static std::unordered_map<CellType::Code, CellType*, std::hash<int>> typeByCode;
	static const CellType POINT1;
	static const CellType SEG2;
	static const CellType SEG3;
	static const CellType SEG4;
	static const CellType POLYL;
	static const CellType TRI3;
	static const CellType QUAD4;
	static const CellType POLYGON;
	static const CellType TRI6;
	static const CellType TRI7;
	static const CellType QUAD8;
	static const CellType QUAD9;
	static const CellType QPOLYG;
	static const CellType TETRA4;
	static const CellType PYRA5;
	static const CellType PENTA6;
	static const CellType HEXA8;
	static const CellType TETRA10;
	static const CellType HEXGP12;
	static const CellType PYRA13;
	static const CellType PENTA15;
	static const CellType HEXA20;
	static const CellType HEXA27;
	static const CellType POLYHED;

private:

	CellType(Code code, int numNodes, SpaceDimension dimension, const std::string& description);
	friend ostream &operator<<(ostream &out, const CellType& cellType); //output

public:
	CellType(const CellType& other);
	Code code;
	unsigned int numNodes;
	SpaceDimension dimension;
	std::string description;
	bool operator==(const CellType& other) const;
	bool operator<(const CellType& other) const;
	//const CellType& operator=(const CellType& other);
	static const CellType* findByCode(Code code);
};

class Mesh;
class NodeStorage;
class CellStorage;

class Group: public Identifiable<Group> {
public:
	enum Type {
		NODE = 0,
		CELL = 1,
		NODEGROUP = 2,
		CELLGROUP = 3
	};
protected:
	Group(Mesh* mesh, std::string name, Type, int id = NO_ORIGINAL_ID);
	Mesh* const mesh;
	std::string name;
public:
	//const SpaceDimension dimension;
	const Type type;
	const std::string getName() const;
	virtual const std::set<int> nodePositions() const = 0;
	virtual ~Group();
};

class NodeGroup final : public Group {
private:
	friend Mesh;
	NodeGroup(Mesh* mesh, const std::string& name, int groupId);
	/**
	 * Positions of the nodes participating to the group
	 */
	std::set<int> _nodePositions;
public:
	/**
	 * Add a node using its numerical id. If the node hasn't been yet defined it reserve a
	 * position in the model.
	 */
	void addNode(int nodeId);
	/**
	 * Add nodes using an iterator to a collection of numerical nodeIds.
	 */
	template<typename iterator>
	void addNodes(iterator begin, iterator end) {
		while (begin != end) {
			addNode(*begin);
			begin++;
		}
	}
	void addNodeByPosition(int nodePosition);
	void removeNodeByPosition(int nodePosition);
	const std::set<int> nodePositions() const override;
	const std::set<int> getNodeIds() const;
};

class Node final {
private:
	friend ostream &operator<<(ostream &out, const Node& node);    //output
	friend Mesh;
	static int auto_node_id;
	Node(int id, double x, double y, double z, int position, DOFS dofs,
			int displacementCS = CoordinateSystem::GLOBAL_COORDINATE_SYSTEM_ID);
public:
	static const int AUTO_ID = INT_MIN;
	const int id;
	const int position;
	const double x;
	const double y;
	const double z;
	//bool, but Valgrind isn't happy maybe gcc 2.7.2 bug?
	const DOFS dofs;
	const int displacementCS;
	const std::string getMedName() const;
	~Node() {
	}
};
/**
 * Identifies a geometry component
 */
class Cell final {
private:
	friend ostream &operator<<(ostream &out, const Cell & cell);    //output
	friend Mesh;
	int findNodeIdPosition(int node_id2) const;
	/**
	 * Every face is identified by the nodes that belongs to that face
	 */
	static const std::unordered_map<CellType::Code, std::vector<std::vector<int>>, std::hash<int> > FACE_BY_CELLTYPE;
	static std::unordered_map<CellType::Code, std::vector<std::vector<int>>, std::hash<int> > init_faceByCelltype();
	static int auto_cell_id;
	/**
	 * @param connectivity
	 * To know the exact meaning of the vector of connectivity.
	 * @see http://www.code-aster.org/outils/med/html/connectivites.html
	 */

	Cell(int id, const CellType &type, const std::vector<int> &nodeIds, const std::vector<int> &nodePositions, bool isvirtual,
			const Orientation*, int elementId, int cellTypePosition);
public:
	static const int AUTO_ID = INT_MIN;
	static const int UNAVAILABLE_ELEM = INT_MIN;
	int id;
	int hasOrientation;
	CellType type;
	std::vector<int> nodeIds;
	std::vector<int> nodePositions;
	bool isvirtual;
	std::shared_ptr<Orientation> orientation;
	int elementId;
	int cellTypePosition;

	/**
	 * @param node1: grid point connected to a corner of the face.
	 * Required data for solid elements only.
	 * @param node2: grid point connected to a corner diagonally
	 * opposite to nodePosition1 on the same face of a CHEXA or CPENTA element.
	 * Required data for quadrilateral faces of CHEXA and CPENTA
	 * elements only. nodePosition2 must be omitted for a triangular surface on a
	 * CPENTA element.
	 *
	 * node2 : CTETRA grid point located at the corner;
	 * this grid point may not reside on the face being loaded. This is
	 * required data and is used for CTETRA elements only.
	 *
	 * <p>For faces of CTETRA elements, nodePosition1 is a corner grid point
	 * that is on the face being loaded and nodePosition2 is a corner grid point
	 * that is not on the face being loaded. Since a CTETRA has only
	 * four corner points, this point nodePosition2 will be unique and
	 * different for each of the four faces of a CTETRA element. </p>
	 */

	std::vector<int> faceids_from_two_nodes(int nodeId1, int nodeId2 = INT_MIN) const;
	/**
	 * Returns the name used in med file for this cell
	 */
	std::string getMedName() const;

	virtual ~Cell();
};

class CellGroup final: public Group {
private:
	friend Mesh;
	CellGroup(Mesh* mesh, std::string name);
public:
	std::unordered_set<int> cellIds;
	void addCell(int cellId);
	std::vector<Cell> getCells();
	std::vector<int> cellPositions();
	const std::set<int> nodePositions() const override;
	virtual ~CellGroup();
};

class NodeIterator final: public std::iterator<std::input_iterator_tag, const Node> {
private:
	const NodeStorage* nodeStorage;
	unsigned int position;
	unsigned int endPosition;
	void increment();
	bool equal(NodeIterator const& other) const;
	friend NodeStorage;
	NodeIterator(const NodeStorage* nodes, int position);
public:
	virtual ~NodeIterator();
	//java style iteration
	bool hasNext() const;
	const Node next();
	NodeIterator& operator++();
	NodeIterator operator++(int);
	bool operator==(const NodeIterator& rhs) const;
	bool operator!=(const NodeIterator& rhs) const;
	const Node operator*();
};

class CellIterator final: public std::iterator<std::input_iterator_tag, const Cell> {
private:
	friend CellStorage;
	const CellStorage* cellStorage;
	unsigned int endPosition;
	CellType cellType;
	unsigned int position;
	bool equal(CellIterator const& other) const;
	void increment(int i);
	const Cell dereference() const;
	friend Mesh;
	CellIterator(const CellStorage* cellStorage, const CellType &cellType, bool begin);
public:
	static const bool POSITION_BEGIN = true;
	static const bool POSITION_END = false;

	virtual ~CellIterator();
	bool hasNext() const;
	const Cell next();
	CellIterator& operator++();
	CellIterator operator++(int);
	bool operator==(const CellIterator& rhs) const;
	bool operator!=(const CellIterator& rhs) const;
	const Cell operator*() const;
};

class NodeContainerMixin final {
protected:
	Mesh* mesh;
	set<int> nodePositions;
	set<int> groupIds;

	NodeContainerMixin(Mesh *mesh);
public:
	class iterator: public std::iterator<std::input_iterator_tag, Node> {
		iterator(int position);
		bool hasNext() const;
		value_type next();
		iterator& operator++();
		iterator operator++(int);
		bool operator==(const iterator& rhs) const;
		bool operator!=(const iterator& rhs) const;
		value_type operator*();
	};
	bool hasNodeGroups();
	std::vector<NodeGroup> getNodeGroups();
	iterator begin();
	iterator end();
};

/**
 * This class represents a container of cells or groups of cells.
 * It can split the groups into single cells.
 *
 */
class CellContainer {
protected:
	std::shared_ptr<Mesh> mesh;
	std::unordered_set<int> cellIds;
	std::unordered_set<std::string> groupNames;
public:
	CellContainer(std::shared_ptr<Mesh> mesh);
	/**
	 * Adds a cellId to the current set
	 */
	void addCell(int cellId);
	void addCellGroup(const std::string& groupName);
	void add(const Cell& cell);
	void add(const CellGroup& cellGroup);
	void add(const CellContainer& cellContainer);
	/**
	 * @param all: if true include also the cells inside all the cellGroups
	 */
	std::vector<Cell> getCells(bool all = false) const;
	/**
	 * Returns the cellIds contained into the Container
	 * @param all: if true include also the cells inside all the cellGroups
	 */
	std::vector<int> getCellIds(bool all = false) const;

	std::set<int> nodePositions() const;

	bool containsCells(CellType cellType, bool all = false);
	/**
	 * True if the container contains some cellGroup
	 */
	bool hasCellGroups() const;
	bool empty() const;
	void clear();
	/**
	 * True if the cellContainer contains some spare cell, not inserted
	 * in any group.
	 */
	bool hasCells() const;
	std::vector<CellGroup *> getCellGroups() const;
	virtual ~CellContainer() {
	}
};

struct Family final {
	std::vector<Group *> groups;
	std::string name;
	int num;
};

class NodeGroup2Families {
	std::vector<Family> families;
	std::vector<int> nodes;
public:
	NodeGroup2Families(int nnodes, const std::vector<NodeGroup *> nodeGroups);
	std::vector<Family>& getFamilies();
	std::vector<int>& getFamilyOnNodes();
};

class CellGroup2Families final {
private:
	std::vector<Family> families;
	std::unordered_map<CellType::Code, std::shared_ptr<std::vector<int>>, std::hash<int>> cellFamiliesByType;
	const Mesh* mesh;
public:
	CellGroup2Families(const Mesh* mesh, std::unordered_map<CellType::Code, int, std::hash<int>> cellCountByType,
			const std::vector<CellGroup *>& cellGroups);
	std::vector<Family>& getFamilies();
	std::unordered_map<CellType::Code, std::shared_ptr<std::vector<int>>, hash<int>>& getFamilyOnCells();
};

} /* namespace vega */

namespace std {

template<>
struct hash<vega::SpaceDimension> {
	size_t operator()(const vega::SpaceDimension& spaceDimension) const {
		return hash<int>()(spaceDimension.code);
	}
};

template<>
struct hash<vega::CellType> {
	size_t operator()(const vega::CellType& cellType) const {
		return hash<int>()(cellType.code);
	}
};

template<>
struct hash<vega::Node> {
	size_t operator()(const vega::Node& node) const {
		return hash<int>()(node.position);
	}
};

} /* namespace std */

#endif /* MESHCOMPONENTS_H_ */
