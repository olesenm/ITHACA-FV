/*---------------------------------------------------------------------------*\
     ██╗████████╗██╗  ██╗ █████╗  ██████╗ █████╗       ███████╗██╗   ██╗
     ██║╚══██╔══╝██║  ██║██╔══██╗██╔════╝██╔══██╗      ██╔════╝██║   ██║
     ██║   ██║   ███████║███████║██║     ███████║█████╗█████╗  ██║   ██║
     ██║   ██║   ██╔══██║██╔══██║██║     ██╔══██║╚════╝██╔══╝  ╚██╗ ██╔╝
     ██║   ██║   ██║  ██║██║  ██║╚██████╗██║  ██║      ██║      ╚████╔╝
     ╚═╝   ╚═╝   ╚═╝  ╚═╝╚═╝  ╚═╝ ╚═════╝╚═╝  ╚═╝      ╚═╝       ╚═══╝

 * In real Time Highly Advanced Computational Applications for Finite Volumes
 * Copyright (C) 2017 by the ITHACA-FV authors
-------------------------------------------------------------------------------

License
    This file is part of ITHACA-FV

    ITHACA-FV is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    ITHACA-FV is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with ITHACA-FV. If not, see <http://www.gnu.org/licenses/>.

Class
    reducedProblem

Description
    A reduced problem for the unsteady NS plus the energy equations for turbulence modelling

SourceFiles
    reducedUnsteadyNSTturb.C

\*---------------------------------------------------------------------------*/

/// \file
/// Header file of the reducedUnsteadyNSTturb class.
/// \dir
/// Directory containing the header and source files for the reducedUnsteadyNSTturb class

#ifndef reducedUnsteadyNSTturb_H
#define reducedUnsteadyNSTturb_H

#include "fvCFD.H"
#include "IOmanip.H"
#include "reducedSteadyNS.H"
#include "reducedUnsteadyNST.H"
#include "unsteadyNSTturb.H"
#include <Eigen/Dense>
#include <unsupported/Eigen/NonLinearOptimization>
#include <unsupported/Eigen/NumericalDiff>

struct newton_unsteadyNSTturb_sup: public newton_argument<double>
{
    public:
        newton_unsteadyNSTturb_sup() {}
        newton_unsteadyNSTturb_sup(int Nx, int Ny,
                                   unsteadyNSTturb& problem): newton_argument<double>(Nx, Ny),
            problem(&problem),
            Nphi_u(problem.NUmodes + problem.liftfield.size() + problem.NSUPmodes),
            Nphi_nut(problem.Nnutmodes),
            Nphi_p(problem.NPmodes),
            N_BC(problem.inletIndex.rows()),
            nu_c(problem.Nnutmodes)
        {}

        int operator()(const Eigen::VectorXd& x, Eigen::VectorXd& fvec) const;
        int df(const Eigen::VectorXd& x,  Eigen::MatrixXd& fjac) const;
        int Nphi_u;
        int Nphi_nut;
        int Nphi_p;
        int N_BC;
        int count_online_solve = 0;
        unsteadyNSTturb* problem;
        Eigen::VectorXd mu_now;
        scalar nu;
        scalar dt;
        Eigen::VectorXd y_old;
        Eigen::VectorXd BC;
        Eigen::VectorXd nu_c;
        std::vector<SPLINTER::RBFSpline*> SPLINES;
};

struct newton_unsteadyNSTturb_sup_t: public newton_argument<double>
{
    public:
        newton_unsteadyNSTturb_sup_t() {}
        newton_unsteadyNSTturb_sup_t(int Nx, int Ny,
                                     unsteadyNSTturb& problem): newton_argument<double>(Nx, Ny),
            problem(&problem),
            Nphi_t(problem.NTmodes + problem.liftfieldT.size()),
            N_BC_t(problem.inletIndexT.rows()),
            nu_c(problem.Nnutmodes)
        {}

        int operator()(const Eigen::VectorXd& x, Eigen::VectorXd& fvec) const;
        int df(const Eigen::VectorXd& x,  Eigen::MatrixXd& fjac) const;
        int Nphi_t;
        int N_BC_t;
        int Nphi_u;
        int Nphi_nut;
        scalar nu;
        scalar DT;
        scalar Prt;
        scalar Pr;
        scalar dt;
        Eigen::VectorXd a_tmp;
        Eigen::MatrixXd Y_matrix;
        Eigen::MatrixXd MT_matrix;
        Eigen::VectorXd mu_now;
        Eigen::VectorXd z;
        Eigen::VectorXd z_old;
        Eigen::VectorXd BC_t;
        Eigen::VectorXd nu_c;
        std::vector<SPLINTER::RBFSpline*> SPLINES;
        unsteadyNSTturb* problem;
};

/*---------------------------------------------------------------------------*\
                        Class reducedProblem Declaration
\*---------------------------------------------------------------------------*/

/// Class where it is implemented a reduced problem for the <b> unsteady Navier-stokes weakly coupled with the energy equation </b> problem.
/** In this class are implemented the methods to solve a reduced version of the non-stationary Navier-stokes coupled with the energy
equations. This class must be used together with the unsteadyNST class */
class reducedUnsteadyNSTturb: public reducedUnsteadyNST
{
    private:

    public:
        // Constructors
        /// Construct Null
        //
        reducedUnsteadyNSTturb();

        /// Construct Null
        ///
        /// @param[in]  problem  a full order unsteadyNS problem
        /// @param[in]  tipo     Type of pressure stabilisation method you want to use "SUP" for supremizer, "PPE" for pressure Poisson equation.
        ///
        explicit reducedUnsteadyNSTturb(unsteadyNSTturb& problem);

        ~reducedUnsteadyNSTturb() {};

        /// List of snapshots for the solution for eddy viscosity
        PtrList<volScalarField> nutFields;

        /// List of POD modes for eddy viscosity
        PtrList<volScalarField> nuTmodes;

        /// Reconstructed eddy viscosity field
        PtrList<volScalarField> nutREC;

        /// Functor object to call the non linear solver sup. approach
        newton_unsteadyNSTturb_sup newton_object_sup;

        /// Functor object to call the non linear solver sup. approach for temperature field
        newton_unsteadyNSTturb_sup_t newton_object_sup_t;

        /// Scalar to store the turbulent Prandtl number
        scalar Prt;

        /// Scalar to store the Prandtl number
        scalar Pr;

        /// Scalar to store the current time
        double time;

        /// Scalar to store the time increment
        double dt;

        /// Scalar to store the final time if the online simulation
        double finalTime;

        /// Scalar to store the final time if the online simulation
        double tstart;

        /// Pointer to the FOM problem
        unsteadyNSTturb* problem;

        /// Number of nut field modes
        int Nphi_nut;

        // Functions
        /// Method to perform an online solve using a supremizer stabilisation method
        ///
        /// @param[in]  vel_now   The vector of online velocity. It is defined in
        /// with an Eigen::MatrixXd and must have one col and as many rows as the number
        /// of parametrized boundary conditions.
        /// @param[in]  temp_now   The vector of online temperature. It is defined in
        /// with an Eigen::MatrixXd and must have one col and as many rows as the number
        /// of parametrized boundary conditions.
        /// @param[in]  startSnap The first snapshot taken from the offline snahpshots
        /// and used to get the reduced initial condition.
        ///
        //void solveOnline_sup(Eigen::MatrixXd, label startSnap=0);
        void solveOnline_sup(Eigen::MatrixXd& vel_now, Eigen::MatrixXd& temp_now,
                             label startSnap = 0);

        /// Method to reconstruct a solution for velocity and pressure from an online solve with a supremizer stabilisation technique.
        /// stabilisation method
        ///
        /// @param[in]  folder      The folder where you want to store the results (default is "./ITHACAOutput/online_rec").
        /// @param[in]  printevery  Variable to recover only every printevery online solutions default is 1.
        ///
        void reconstruct_sup(fileName folder = "./ITHACAOutput/online_rec",
                             int printevery = 1);

        /// Method to reconstruct a solution for temperature from an online solve with a supremizer stabilisation technique.
        /// stabilisation method
        ///
        /// @param[in]  folder      The folder where you want to store the results (default is "./ITHACAOutput/online_rec").
        /// @param[in]  printevery  Variable to recover only every printevery online solutions default is 1.
        ///
        void reconstruct_supt(fileName folder = "./ITHACAOutput/online_rec",
                              int printevery = 1);
};

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif






