/*
 * Copyright (C) Alneos, s. a r. l. (contact@alneos.fr) 
 * Unauthorized copying of this file, via any medium is strictly prohibited. 
 * Proprietary and confidential.
 *
 * Analysis.cpp
 *
 *  Created on: Sep 4, 2013
 *      Author: devel
 */

#ifdef __GNUC__
#include <valgrind/memcheck.h>
#endif
#include "Analysis.h"
#include "Model.h"
#include "BoundaryCondition.h"

namespace vega {

Analysis::Analysis(Model& model, const Type Type, int original_id) :
		Identifiable(original_id), model(model), type(Type) {
}

const string Analysis::name = "Analysis";

const map<Analysis::Type, string> Analysis::stringByType = {
		{ LINEAR_MECA_STAT, "LINEAR_MECA_STAT" },
		{ LINEAR_MODAL, "LINEAR_MODAL" },
		{ LINEAR_DYNA_MODAL_FREQ, "LINEAR_DYNA_MODAL_FREQ" },
		{ NONLINEAR_MECA_STAT, "NONLINEAR_MECA_STAT" },
		{ UNKNOWN, "UNKNOWN" }
};

ostream &operator<<(ostream &out, const Analysis& analysis) {
	out << to_str(analysis);
	return out;
}

void Analysis::add(const Reference<LoadSet>& loadSetReference) {
	this->loadSet_references.push_back(loadSetReference.clone());
}

const vector<shared_ptr<LoadSet>> Analysis::getLoadSets() const {
	vector<shared_ptr<LoadSet>> result;
	shared_ptr<LoadSet> commonLoadSet = model.find(model.commonLoadSet.getReference());
	if (commonLoadSet)
		result.push_back(commonLoadSet);
	for (auto &loadSetReference : this->loadSet_references) {
		result.push_back(model.find(*loadSetReference));
	}
	return result;
}

const vector<shared_ptr<BoundaryCondition>> Analysis::getBoundaryConditions() const {
	vector<shared_ptr<BoundaryCondition>> result;
	for (shared_ptr<ConstraintSet> constraintSet : getConstraintSets()) {
		if (constraintSet == nullptr) {
			continue;
		}
		for (auto& constraint : constraintSet->getConstraints()) {
			if (constraint != nullptr) {
				result.push_back(constraint);
			}
		}
	}
	for (shared_ptr<LoadSet> loadSet : getLoadSets()) {
		if (loadSet == nullptr) {
			continue;
		}
		for (auto& loading : loadSet->getLoadings()) {
			if (loading != nullptr) {
				result.push_back(loading);
			}
		}
	}
	return result;
}

void Analysis::add(const Reference<ConstraintSet>& constraintSetReference) {
	this->constraintSet_references.push_back(constraintSetReference.clone());
}

void Analysis::remove(const Reference<LoadSet> loadSetReference) {
	for (auto it = loadSet_references.begin(); it != loadSet_references.end(); ++it) {
		if (**it == loadSetReference) {
			loadSet_references.erase(it);
			return;
		}
	}
}

void Analysis::remove(const Reference<ConstraintSet> constraintSetReference) {
	for (auto it = constraintSet_references.begin(); it != constraintSet_references.end(); ++it) {
		if (**it == constraintSetReference) {
			constraintSet_references.erase(it);
			return;
		}
	}
}

void Analysis::remove(const Reference<Objective> objectiveReference) {
	for (auto it = assertion_references.begin(); it != assertion_references.end(); ++it) {
		if (**it == objectiveReference) {
			assertion_references.erase(it);
			return;
		}
	}
}

bool Analysis::contains(const Reference<LoadSet> reference) const {
	for (auto loadset_ref_ptr : loadSet_references) {
		if (reference == *loadset_ref_ptr) {
			return true;
		}
	}
	return false;
}

bool Analysis::contains(const Reference<ConstraintSet> reference) const {
	for (auto constraintset_ref_ptr : constraintSet_references) {
		if (reference == *constraintset_ref_ptr) {
			return true;
		}
	}
	return false;
}

bool Analysis::contains(const Reference<Objective> reference) const {
	for (auto assertion_ref_ptr : assertion_references) {
		if (reference == *assertion_ref_ptr) {
			return true;
		}
	}
	return false;
}

const vector<shared_ptr<ConstraintSet>> Analysis::getConstraintSets() const {
	vector<shared_ptr<ConstraintSet>> result;
	shared_ptr<ConstraintSet> commonConstraintSet = model.find(
			model.commonConstraintSet.getReference());
	if (commonConstraintSet)
		result.push_back(commonConstraintSet);
	for (auto &constraintSetReference : this->constraintSet_references) {
		result.push_back(model.find(*constraintSetReference));
	}

	return result;
}

void Analysis::add(const Reference<Objective>& assertionReference) {
	assertion_references.push_back(assertionReference.clone());
}

const vector<shared_ptr<Assertion>> Analysis::getAssertions() const {
	vector<shared_ptr<Assertion>> assertions;
	for (auto assertion_reference : assertion_references) {
		shared_ptr<Objective> objective = model.find(*assertion_reference);
			assertions.push_back(dynamic_pointer_cast<Assertion>(objective));
		}
	return assertions;
}

bool Analysis::validate() const {
	vector<shared_ptr<ConstraintSet>> constraintSets = getConstraintSets();
	if (find(constraintSets.begin(), constraintSets.end(), shared_ptr<ConstraintSet>())
			!= constraintSets.end())
		return false;
	vector<shared_ptr<LoadSet>> loadSets = getLoadSets();
	if (find(loadSets.begin(), loadSets.end(), shared_ptr<LoadSet>()) != loadSets.end())
		return false;
	return true;
}


void Analysis::removeSPCNodeDofs(SinglePointConstraint& spc, int nodePosition,	const DOFS dofsToRemove) {
	DOFS remainingDofs = spc.getDOFSForNode(nodePosition) - dofsToRemove;
	Node node = model.mesh->findNode(nodePosition);
	set<shared_ptr<ConstraintSet>> affectedConstraintSets =
			model.getConstraintSetsByConstraint(
					spc);
	if (remainingDofs.size() != 0) {
		SinglePointConstraint remainingSpc = SinglePointConstraint(this->model);
		remainingSpc.addNodeId(node.id);
		for (DOF remainingDof : remainingDofs) {
			remainingSpc.setDOF(remainingDof, spc.getDoubleForDOF(remainingDof));
		}
		model.add(remainingSpc);
		if (model.configuration.logLevel >= LogLevel::DEBUG) {
			cout << "Created new spc : " << remainingSpc << " for node position : "
					<< nodePosition
					<< " to handle dofs : " << remainingDofs << endl;
		}
		for (const auto& constraintSet : affectedConstraintSets) {
			model.addConstraintIntoConstraintSet(remainingSpc, *constraintSet);
		}
	}
	if (model.analyses.size() >= 2) {
		ConstraintSet otherAnalysesCS(model, ConstraintSet::SPC);
		model.add(otherAnalysesCS);
		SinglePointConstraint otherAnalysesSpc = SinglePointConstraint(this->model);
		otherAnalysesSpc.addNodeId(node.id);
		for (DOF removedDof : dofsToRemove) {
			otherAnalysesSpc.setDOF(removedDof, spc.getDoubleForDOF(removedDof));
		}
		model.add(otherAnalysesSpc);
		model.addConstraintIntoConstraintSet(otherAnalysesSpc, otherAnalysesCS);
		if (this->model.configuration.logLevel >= LogLevel::DEBUG) {
			cout << "Created new spc : " << otherAnalysesSpc << " for node position : "
					<< nodePosition
					<< " to handle dofs : " << dofsToRemove << endl;
		}
		for (auto otherAnalysis : this->model.analyses) {
			if (*this == *otherAnalysis) {
				continue;
			}
			for (const auto& constraintSet : otherAnalysis->getConstraintSets()) {
				if (affectedConstraintSets.find(constraintSet) == affectedConstraintSets.end()) {
					continue;
				}
				otherAnalysis->add(otherAnalysesCS);
				break;
			}
		}
	}
	spc.removeNode(nodePosition);
}

void Analysis::addBoundaryDOFS(int nodePosition, const DOFS dofs) {
	boundaryDOFSByNodePosition[nodePosition] = DOFS(boundaryDOFSByNodePosition[nodePosition]) + dofs;
}

const DOFS Analysis::findBoundaryDOFS(int nodePosition) const {
	const auto& entry = boundaryDOFSByNodePosition.find(nodePosition);
	if (entry == boundaryDOFSByNodePosition.end()) {
		return DOFS::NO_DOFS;
	} else {
		return entry->second;
	}
}

const set<int> Analysis::boundaryNodePositions() const {
	set<int> result;
	for (auto& entry : boundaryDOFSByNodePosition) {
		result.insert(entry.first);
	}
	return result;
}

Analysis::~Analysis() {
}

LinearMecaStat::LinearMecaStat(Model& model, const int original_id) :
		Analysis(model, Analysis::LINEAR_MECA_STAT, original_id) {

}

shared_ptr<Analysis> LinearMecaStat::clone() const {
	return shared_ptr<LinearMecaStat>(new LinearMecaStat(*this));
}

NonLinearMecaStat::NonLinearMecaStat(Model& model, const int strategy_id, const int original_id) :
		Analysis(model, Analysis::NONLINEAR_MECA_STAT, original_id), strategy_reference(
				Objective::NONLINEAR_STRATEGY, strategy_id) {

}

shared_ptr<Analysis> NonLinearMecaStat::clone() const {
	return shared_ptr<NonLinearMecaStat>(new NonLinearMecaStat(*this));
}

bool NonLinearMecaStat::validate() const {
	return Analysis::validate();
}

LinearModal::LinearModal(Model& model, const int frequency_band_original_id, const int original_id,
		const Type type) :
		Analysis(model, type, original_id), frequency_band_reference(Objective::FREQUENCY_BAND,
				frequency_band_original_id) {
}

shared_ptr<FrequencyBand> LinearModal::getFrequencyBand() const {
	return dynamic_pointer_cast<FrequencyBand>(model.find(frequency_band_reference));
}

shared_ptr<Analysis> LinearModal::clone() const {
	return shared_ptr<LinearModal>(new LinearModal(*this));
}

bool LinearModal::validate() const {
	return Analysis::validate() && getFrequencyBand();
}

LinearDynaModalFreq::LinearDynaModalFreq(Model& model, const int frequency_band_original_id,
		const int modal_damping_original_id, const int frequency_value_original_id,
		const bool residual_vector, const int original_id) :
		LinearModal(model, frequency_band_original_id, original_id,
				Analysis::LINEAR_DYNA_MODAL_FREQ), modal_damping_reference(Objective::MODAL_DAMPING,
				modal_damping_original_id), frequency_values_reference(Objective::FREQUENCY_TARGET,
				frequency_value_original_id), residual_vector(residual_vector) {
}

shared_ptr<ModalDamping> LinearDynaModalFreq::getModalDamping() const {
	return dynamic_pointer_cast<ModalDamping>(model.find(modal_damping_reference));
}

shared_ptr<FrequencyValues> LinearDynaModalFreq::getFrequencyValues() const {
	return dynamic_pointer_cast<FrequencyValues>(model.find(frequency_values_reference));
}

shared_ptr<Analysis> LinearDynaModalFreq::clone() const {
	return shared_ptr<LinearDynaModalFreq>(new LinearDynaModalFreq(*this));
}

bool LinearDynaModalFreq::validate() const {
	return LinearModal::validate() && getModalDamping() && getFrequencyValues();
}

} /* namespace vega */
