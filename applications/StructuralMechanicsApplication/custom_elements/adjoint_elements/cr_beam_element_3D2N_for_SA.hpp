// ==============================================================================
//  KratosStructuralMechanicsApplication
//
//  License:         BSD License
//                   license: StructuralMechanicsApplication/license.txt
//
//  Main authors:    Fusseder Martin   
//                   martin.fusseder@tum.de
//
// ==============================================================================

#if !defined(KRATOS_CR_BEAM_ELEMENT_3D2N_FOR_SA_H_INCLUDED )
#define  KRATOS_CR_BEAM_ELEMENT_3D2N_FOR_SA_H_INCLUDED


#include "includes/element.h"
#include "includes/define.h"
#include "includes/variables.h"
#include "includes/serializer.h"

namespace Kratos
{

	class CrBeamElement3D2NForSA : public Element
	{
	public:
		KRATOS_CLASS_POINTER_DEFINITION(CrBeamElement3D2NForSA);


		typedef Element BaseType;
		typedef BaseType::GeometryType GeometryType;
		typedef BaseType::NodesArrayType NodesArrayType;
		typedef BaseType::PropertiesType PropertiesType;
		typedef BaseType::IndexType IndexType;
		typedef BaseType::SizeType SizeType;
		typedef BaseType::MatrixType MatrixType;
		typedef BaseType::VectorType VectorType;
		typedef BaseType::EquationIdVectorType EquationIdVectorType;
		typedef BaseType::DofsVectorType DofsVectorType;

		typedef Variable<int> VariableIntType;
		typedef Variable<std::string> VariableStringType;
		typedef Variable<double> VariableDoubleType;
		typedef Variable<Vector> VariableVectorType;


		CrBeamElement3D2NForSA(IndexType NewId, GeometryType::Pointer pGeometry,
						bool rLinear = false);
		CrBeamElement3D2NForSA(IndexType NewId, GeometryType::Pointer pGeometry,
						PropertiesType::Pointer pProperties,
						bool rLinear = false);


		~CrBeamElement3D2NForSA() override;


		BaseType::Pointer Create(
			IndexType NewId,
			NodesArrayType const& rThisNodes,
			PropertiesType::Pointer pProperties) const override;

		// new create by fusseder	
		Element::Pointer Create(IndexType NewId,
            GeometryType::Pointer pGeom,
            PropertiesType::Pointer pProperties) const override;

		void EquationIdVector(
			EquationIdVectorType& rResult,
			ProcessInfo& rCurrentProcessInfo) override;

		void GetDofList(
			DofsVectorType& rElementalDofList,
			ProcessInfo& rCurrentProcessInfo) override;

		void Initialize() override;

		//additional stuff for sensitivity analysis------------------------------------------------------------------
		void Calculate(const Variable<double>& rVariable, double& Output,const ProcessInfo& rCurrentProcessInfo);

		void Calculate(const Variable<Vector >& rVariable, Vector& Output, const ProcessInfo& rCurrentProcessInfo);

		void compute_adjoint_load(Vector& Output);

		void compute_zero_adjoint_load(Vector& Output);

		int get_id_in_stress_vector();

		void CalculateSensitivityMatrix(const Variable<double>& rDesignVariable, Matrix& rOutput, 
											const ProcessInfo& rCurrentProcessInfo);
		
		void CalculateSensitivityMatrix(const Variable<array_1d<double,3>>& rDesignVariable, Matrix& rOutput, 
											const ProcessInfo& rCurrentProcessInfo);
		//-----------------------------------------------------------------------------------------------------------
		
		Matrix CreateElementStiffnessMatrix_Material();
		Matrix CreateElementStiffnessMatrix_Geometry(const Vector qe);
		Matrix CalculateDeformationStiffness();
		Matrix CalculateTransformationS();

		Vector CalculateElementForces();

		void CalculateTransformationMatrix(
			Matrix& rRotationMatrix);

		void CalculateInitialLocalCS();

		Matrix UpdateRotationMatrixLocal();

		void CalculateLocalSystem(
			MatrixType& rLeftHandSideMatrix,
			VectorType& rRightHandSideVector,
			ProcessInfo& rCurrentProcessInfo) override;

		void CalculateRightHandSide(
			VectorType& rRightHandSideVector,
			ProcessInfo& rCurrentProcessInfo) override;

		void CalculateLeftHandSide(
			MatrixType& rLeftHandSideMatrix,
			ProcessInfo& rCurrentProcessInfo) override;

		void CalculateMassMatrix(
			MatrixType& rMassMatrix,
			ProcessInfo& rCurrentProcessInfo) override;

		void CalculateLumpedMassMatrix(
			MatrixType& rMassMatrix,
			ProcessInfo& rCurrentProcessInfo);

		void CalculateConsistentMassMatrix(
			MatrixType& rMassMatrix,
			ProcessInfo& rCurrentProcessInfo);

		void BuildSingleMassMatrix(
			MatrixType& rMassMatrix,
			const double Phi, const double CT, const double CR, const double L);

		void CalculateDampingMatrix(
			MatrixType& rDampingMatrix,
			ProcessInfo& rCurrentProcessInfo) override;

		void AddExplicitContribution(const VectorType& rRHSVector,
			const Variable<VectorType>& rRHSVariable,
			Variable<array_1d<double, 3> >& rDestinationVariable,
			const ProcessInfo& rCurrentProcessInfo) override;

		void GetValuesVector(
			Vector& rValues,
			int Step = 0) override;

		// special function for sensitivity analysis to get the values of the primal solution	
		void GetPrimalValuesVector(
			Vector& rValues,
			int Step = 0);	

		void GetSecondDerivativesVector(
			Vector& rValues,
			int Step = 0) override;

		void GetFirstDerivativesVector(
			Vector& rValues,
			int Step = 0) override;

		void AssembleSmallInBigMatrix(Matrix SmallMatrix, Matrix& BigMatrix);

		int Check(const ProcessInfo& rCurrentProcessInfo) override;


		double CalculateCurrentLength();
		double CalculatePsi(const double I, const double A_eff);
		double CalculateShearModulus();
		double CalculateReferenceLength();
		void UpdateIncrementDeformation();

		Vector CalculateBodyForces();  

		void CalculateOnIntegrationPoints(
			const Variable<array_1d<double, 3 > >& rVariable,
			std::vector< array_1d<double, 3 > >& rOutput,
			const ProcessInfo& rCurrentProcessInfo) override;

		void GetValueOnIntegrationPoints(
			const Variable<array_1d<double, 3 > >& rVariable,
			std::vector< array_1d<double, 3 > >& rOutput,
			const ProcessInfo& rCurrentProcessInfo) override;

		void CalculateOnIntegrationPoints(
			const Variable<Vector >& rVariable,
			std::vector< Vector >& rOutput,
			const ProcessInfo& rCurrentProcessInfo) override;

		void GetValueOnIntegrationPoints(
			const Variable<Vector>& rVariable,
			std::vector<Vector>& rValues,
			const ProcessInfo& rCurrentProcessInfo) override;

		void CalculateOnIntegrationPoints(const Variable<double>& rVariable,
					      std::vector<double>& rOutput,
					      const ProcessInfo& rCurrentProcessInfo) override;

		void GetValueOnIntegrationPoints(const Variable<double>& rVariable,
					     std::vector<double>& rValues,
					     const ProcessInfo& rCurrentProcessInfo) override;				  

		IntegrationMethod GetIntegrationMethod() const override;

		void CalculateAndAddWorkEquivalentNodalForcesLineLoad(
			const Vector ForceInput, VectorType& rRightHandSideVector,
			const double GeometryLength);

	private:
		double mdPhi_x_a, mRotInertiaY, mRotInertiaZ;
		Vector mNX, mNY, mNZ, mRHS, mTotalDef, mTotalPos;
		Vector mTotalNodalDeformation, mTotalNodalPosistion, mBodyForces;
		Vector mDeformationModes, mIncrementDeformation;
		Matrix mLHS, mRotationMatrix, mRotationMatrix0;
		Vector mNX0, mNY0, mNZ0;
		Vector mQuaternionVEC_A, mQuaternionVEC_B;
		double mQuaternionSCA_A, mQuaternionSCA_B;
		Vector mPhiS, mPhiA;
		Vector mNodalForces;

		int mIterationCount = 0;
		bool mIsLinearElement = false;
		bool mIsLumpedMassMatrix = false;

		CrBeamElement3D2NForSA() {};



		friend class Serializer;
		void save(Serializer& rSerializer) const override;
		void load(Serializer& rSerializer) override;
	};



}

#endif