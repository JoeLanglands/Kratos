import KratosMultiphysics
import KratosMultiphysics.ExternalSolversApplication
import KratosMultiphysics.FluidDynamicsApplication as KratosFluid

import KratosMultiphysics.KratosUnittest as UnitTest

import os
import math

class WorkFolderScope:
    def __init__(self, work_folder):
        self.currentPath = os.getcwd()
        self.scope = os.path.abspath(os.path.join(os.path.dirname(os.path.realpath(__file__)),work_folder))

    def __enter__(self):
        os.chdir(self.scope)

    def __exit__(self, type, value, traceback):
        os.chdir(self.currentPath)

class EmbeddedCircularFlowTest(UnitTest.TestCase):
    def testEmbeddedCircularFlow2D(self):
        self.distance = 0.5
        self.embedded_formulation = "no_slip"
        self.work_folder = "EmbeddedCircularFlow2DTest"   
        self.reference_file = "reference_no_slip_circular_flow_2D"
        self.settings = "EmbeddedCircularFlow2DTest_parameters.json"

        with WorkFolderScope(self.work_folder):
            self.setUp()
            self.setUpProblem()
            self.setUpDistanceField()
            self.runTest()
            self.tearDown()
            self.checkResults()

    def testEmbeddedSlipCircularFlow2D(self):
        self.distance = 0.5
        self.embedded_formulation = "slip"
        self.work_folder = "EmbeddedCircularFlow2DTest"   
        self.reference_file = "reference_slip_circular_flow_2D"
        self.settings = "EmbeddedCircularFlow2DTest_parameters.json"

        with WorkFolderScope(self.work_folder):
            self.setUp()
            self.setUpProblem()
            self.setUpDistanceField()
            self.runTest()
            self.tearDown()
            self.checkResults()

    def testEmbeddedAusasSlipCircularFlow2D(self):
        self.distance = 0.5
        self.embedded_formulation = "ausas_slip"
        self.work_folder = "EmbeddedCircularFlow2DTest"   
        self.reference_file = "reference_ausas_slip_circular_flow_2D"
        self.settings = "EmbeddedCircularFlow2DTest_parameters.json"

        with WorkFolderScope(self.work_folder):
            self.setUp()
            self.setUpProblem()
            self.setUpDistanceField()
            self.runTest()
            self.tearDown()
            self.checkResults()

    def setUp(self):
        self.check_tolerance = 1e-6
        self.print_output = False
        self.print_reference_values = False

    def tearDown(self):
        with WorkFolderScope(self.work_folder):
            try:
                os.remove(self.ProjectParameters["solver_settings"]["model_import_settings"]["input_filename"].GetString()+'.time')
            except FileNotFoundError as e:
                pass

    def setUpProblem(self):
        with WorkFolderScope(self.work_folder):
            parameter_file = open(self.settings, 'r')

            self.ProjectParameters = KratosMultiphysics.Parameters(parameter_file.read())

            self.main_model_part = KratosMultiphysics.ModelPart(self.ProjectParameters["problem_data"]["model_part_name"].GetString())
            self.main_model_part.ProcessInfo.SetValue(KratosMultiphysics.DOMAIN_SIZE, self.ProjectParameters["problem_data"]["domain_size"].GetInt())

            Model = {self.ProjectParameters["problem_data"]["model_part_name"].GetString() : self.main_model_part}

            ## Set the solver type according to the embedded formulation
            if ((self.embedded_formulation == "no_slip") or (self.embedded_formulation == "slip")):
                self.ProjectParameters["solver_settings"]["solver_type"].SetString("Embedded")
            elif (self.embedded_formulation == "ausas_slip"):
                self.ProjectParameters["solver_settings"]["solver_type"].SetString("EmbeddedAusas")
            else:
                raise Exception("Wrong solver type in embedded reservoir test settings.")

            ## Set the distance modification settings according to the embedded formulation
            ## Note that process 2 corresponds to the distance modification one in the settings Json file
            if ((self.embedded_formulation == "no_slip") or (self.embedded_formulation == "slip")):
                self.ProjectParameters["auxiliar_process_list"][0]["Parameters"]["deactivate_full_negative_elements"].SetBool(True)
            elif (self.embedded_formulation == "ausas_slip"):
                self.ProjectParameters["auxiliar_process_list"][0]["Parameters"]["deactivate_full_negative_elements"].SetBool(False)

            ## Solver construction
            import python_solvers_wrapper_fluid
            self.solver = python_solvers_wrapper_fluid.CreateSolver(self.main_model_part, self.ProjectParameters)

            self.solver.AddVariables()

            ## Read the model - note that SetBufferSize is done here
            self.solver.ImportModelPart()

            ## Add AddDofs
            self.solver.AddDofs()

            ## Solver initialization
            self.solver.Initialize()

            ## Get the list of the skin submodel parts in the object Model
            for i in range(self.ProjectParameters["solver_settings"]["skin_parts"].size()):
                skin_part_name = self.ProjectParameters["solver_settings"]["skin_parts"][i].GetString()
                Model.update({skin_part_name: self.main_model_part.GetSubModelPart(skin_part_name)})

            ## Get the gravity submodel part in the object Model
            for i in range(self.ProjectParameters["gravity"].size()):
                gravity_part_name = self.ProjectParameters["gravity"][i]["Parameters"]["model_part_name"].GetString()
                Model.update({gravity_part_name: self.main_model_part.GetSubModelPart(gravity_part_name)})

            ## Processes construction
            import process_factory
            self.list_of_processes  = process_factory.KratosProcessFactory(Model).ConstructListOfProcesses( self.ProjectParameters["gravity"] )
            self.list_of_processes += process_factory.KratosProcessFactory(Model).ConstructListOfProcesses( self.ProjectParameters["boundary_conditions_process_list"] )
            self.list_of_processes += process_factory.KratosProcessFactory(Model).ConstructListOfProcesses( self.ProjectParameters["auxiliar_process_list"] )

            ## Processes initialization
            for process in self.list_of_processes:
                process.ExecuteInitialize()

    def setUpDistanceField(self):
        # Set the distance function
        for node in self.main_model_part.Nodes:
            distance = math.sqrt(node.X**2 + node.Y**2) - self.distance
            node.SetSolutionStepValue(KratosMultiphysics.DISTANCE, 0, distance)

        # Set the ELEMENTAL_DISTANCES value
        n_nodes = len(self.main_model_part.Elements[1].GetNodes())
        for element in self.main_model_part.Elements:
            elem_dist = KratosMultiphysics.Vector(n_nodes)
            elem_nodes = element.GetNodes()
            for i_node in range(0,n_nodes):
                elem_dist[i_node] = elem_nodes[i_node].GetSolutionStepValue(KratosMultiphysics.DISTANCE)
            element.SetValue(KratosMultiphysics.ELEMENTAL_DISTANCES, elem_dist)

        # If proceeds, set the SLIP flag
        if ((self.embedded_formulation == "slip") or (self.embedded_formulation == "ausas_slip")):
            KratosMultiphysics.VariableUtils().SetFlag(KratosMultiphysics.SLIP, True, self.main_model_part.Elements)

    def runTest(self):
        with WorkFolderScope(self.work_folder):
            if (self.print_output):
                gid_mode = KratosMultiphysics.GiDPostMode.GiD_PostBinary
                multifile = KratosMultiphysics.MultiFileFlag.SingleFile
                deformed_mesh_flag = KratosMultiphysics.WriteDeformedMeshFlag.WriteUndeformed
                write_conditions = KratosMultiphysics.WriteConditionsFlag.WriteElementsOnly
                gid_io = KratosMultiphysics.GidIO(self.ProjectParameters["solver_settings"]["model_import_settings"]["input_filename"].GetString(),gid_mode,multifile,deformed_mesh_flag, write_conditions)

                mesh_name = 0.0
                gid_io.InitializeMesh( mesh_name)
                gid_io.WriteMesh( self.main_model_part.GetMesh() )
                gid_io.FinalizeMesh()
                gid_io.InitializeResults(mesh_name,(self.main_model_part).GetMesh())

            end_time = self.ProjectParameters["problem_data"]["end_time"].GetDouble()

            time = 0.0
            step = 0

            for process in self.list_of_processes:
                process.ExecuteBeforeSolutionLoop()

            while(time <= end_time):

                Dt = self.solver.ComputeDeltaTime()
                step += 1
                time += Dt
                self.main_model_part.CloneTimeStep(time)
                self.main_model_part.ProcessInfo[KratosMultiphysics.STEP] = step

                for process in self.list_of_processes:
                    process.ExecuteInitializeSolutionStep()

                self.solver.Solve()

                for process in self.list_of_processes:
                    process.ExecuteFinalizeSolutionStep()

                for process in self.list_of_processes:
                    process.ExecuteBeforeOutputStep()

                if (self.print_output):
                    gid_io.WriteNodalResults(KratosMultiphysics.VELOCITY,self.main_model_part.Nodes,time,0)
                    gid_io.WriteNodalResults(KratosMultiphysics.PRESSURE,self.main_model_part.Nodes,time,0)
                    gid_io.WriteNodalResults(KratosMultiphysics.DISTANCE,self.main_model_part.Nodes,time,0)
                    gid_io.WriteNodalResults(KratosFluid.EMBEDDED_WET_VELOCITY,self.main_model_part.Nodes,time,0)

                for process in self.list_of_processes:
                    process.ExecuteAfterOutputStep()

            for process in self.list_of_processes:
                process.ExecuteFinalize()

            if (self.print_output):
                gid_io.FinalizeResults()

    def checkResults(self):
        with WorkFolderScope(self.work_folder):
            if self.print_reference_values:
                with open(self.reference_file+'.csv','w') as ref_file:
                    ref_file.write("#ID, PRESSURE\n")
                    for node in self.main_model_part.Nodes:
                        pres = node.GetSolutionStepValue(KratosMultiphysics.PRESSURE)
                        ref_file.write("{0}, {1}\n".format(node.Id, pres))
            else:
                with open(self.reference_file+'.csv','r') as reference_file:
                    reference_file.readline() # skip header
                    line = reference_file.readline()

                    for node in self.main_model_part.Nodes:
                        values = [ float(i) for i in line.rstrip('\n ').split(',') ]
                        node_id = values[0]
                        reference_pres = values[1]

                        pres = node.GetSolutionStepValue(KratosMultiphysics.PRESSURE)
                        self.assertAlmostEqual(reference_pres, pres, delta = self.check_tolerance)

                        line = reference_file.readline()
                    if line != '': # If we did not reach the end of the reference file
                        self.fail("The number of nodes in the mdpa is smaller than the number of nodes in the output file")

if __name__ == '__main__':
    test = EmbeddedCircularFlowTest()
    test.setUp()
    test.distance = 0.5
    test.print_output = True
    test.print_reference_values = False
    test.embedded_formulation = "slip"
    test.work_folder = "EmbeddedCircularFlow2DTest"
    test.reference_file = "reference_" + test.embedded_formulation + "_circular_flow_2D"   
    test.settings = "EmbeddedCircularFlow2DTest_parameters.json"
    test.setUpProblem()
    test.setUpDistanceField()
    test.runTest()
    test.tearDown()
    test.checkResults()
    