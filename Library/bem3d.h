/* ------------------------------------------------------------
 This is the file "bem3d.h" of the H2Lib package.
 All rights reserved, Sven Christophersen 2011
 ------------------------------------------------------------ */

/**
 * @file bem3d.h
 * @author Sven Christophersen
 * @date 2011
 */

#ifndef BEM3D_H_
#define BEM3D_H_

/* C STD LIBRARY */
#include <stdio.h>
/* CORE 0 */
#include "basic.h"
/* CORE 1 */
#include "amatrix.h"
#include "gaussquad.h"
#include "factorizations.h"
#include "krylov.h"
/* CORE 2 */
#include "cluster.h"
#include "clustergeometry.h"
#include "hmatrix.h"
#include "clusterbasis.h"
#include "h2matrix.h"
/* CORE 3 */
#include "hcoarsen.h"
#include "h2update.h"
#include "aca.h"
/* SIMPLE */
/* PARTICLES */
/* BEM */
#include "macrosurface3d.h"
#include "surface3d.h"
#include "singquad2d.h"

/** \defgroup bem3d bem3d
 *  @brief This module contains main algorithms to solve boundary element method
 *  problems in 3 dimensional space.
 *
 *  @ref bem3d provides basic functionality for a general BEM-application.
 *  Considering a special problem such as the Laplace-equation one has to include
 *  the @ref laplacebem3d module, which provides the essential quadrature routines of
 *  the kernel functions.
 *  The @ref bem3d module consists of a struct called @ref _bem3d "bem3d", which
 *  contains a collection of callback functions, that regulates the whole computation
 *  of fully-populated BEM-matrices and also of @ref _hmatrix "h-"
 *  and @ref _h2matrix "h2-matrices".
 *  @{ */

/* ------------------------------------------------------------ *
 * Type definitions                                             *
 * ------------------------------------------------------------ */

/**
 * @ref bem3d is just an abbreviation for the struct @ref _bem3d ,
 * which contains all necessary information to solve BEM-problems.
 */
typedef struct _bem3d bem3d;
/**
 * Pointer to a @ref bem3d object.
 */
typedef bem3d *pbem3d;
/**
 * Pointer to a constant @ref bem3d object.
 */
typedef const bem3d* pcbem3d;

/**
 * @ref aprxbem3d is just an abbreviation for the struct _aprxbem3d hidden inside
 * the bem3d.c file.
 * This struct is necessary because of the different approximation
 * techniques offered by this library. It will hide all dispensable information
 * from the user.
 */
typedef struct _aprxbem3d aprxbem3d;
/**
 * Pointer to a @ref aprxbem3d object.
 */
typedef aprxbem3d *paprxbem3d;
/**
 * Pointer to a constant @ref aprxbem3d object.
 */
typedef const aprxbem3d *pcaprxbem3d;

/**
 * @ref parbem3d is just an abbreviation for the struct _parbem3d , which
 * is hidden inside the bem3d.c . It is necessary for parallel computation
 * on @ref _hmatrix "h-" and @ref _h2matrix "h2matrices".
 */
typedef struct _parbem3d parbem3d;
/**
 * Pointer to a @ref parbem3d object.
 */
typedef parbem3d *pparbem3d;
/**
 * Pointer to a constant @ref parbem3d object.
 */
typedef const parbem3d *pcparbem3d;

/**
 * @ref kernelbem3d is just an abbreviation for the struct _kernelbem3d.
 * It contains a collection of callback functions to evaluate or integrate
 * kernel function and its derivatives.
 */
typedef struct _kernelbem3d kernelbem3d;

/**
 * Pointer to a @ref _kernelbem3d "kernelbem3d" object.
 */
typedef kernelbem3d *pkernelbem3d;

/**
 * Pointer to a constant @ref _kernelbem3d "kernelbem3d" object.
 */
typedef const kernelbem3d *pckernelbem3d;

/**
 * @brief Callback function type for parameterizations.
 *
 * @ref quadpoints3d defines a class of callback functions that are used to
 * compute quadrature points, weight and normal vectors corresponding to the
 * points on a given parameterization, which are used within the green's
 * approximation functions. The parameters are in ascending order:
 * @param bem Reference to the bem3d object.
 * @param bmin A 3D-array determining the minimal spatial dimensions of the
 * bounding box the parameterization is located around.
 * @param bmax A 3D-array determining the maximal spatial dimensions of the
 * bounding box the parameterization is located around.
 * @param delta Defines the distant between bounding box and the parameterization.
 * @param Z The quadrature points will be stored inside of Z .
 * @param N The outer normal vectors will be stored inside of N . For a
 * quadrature point @f$ z_i @f$ , a corresponding quadrature weight @f$ \omega_i @f$
 * and the proper normal vector @f$ \vec{n_i} @f$ the vector will not have unit length
 * but @f$ \lVert \vec{n_i} \rVert = \omega_i @f$ .
 */
typedef void (*quadpoints3d)(pcbem3d bem, const real bmin[3],
    const real bmax[3], const real delta, real (**Z)[3], real (**N)[3]);

/**
 * @brief Possible types of basis functions.
 *
 * This enum defines all possible values usable for basis functions for either
 * neumann- and dirichlet-data.
 * The values should be self-explanatory.
 */
enum _basisfunctionbem3d {
  /**
   * @brief Dummy value, should never be used.
   */
  BASIS_NONE_BEM3D = 0,
  /**
   * @brief Enum value to represent piecewise constant basis functions.
   */
  BASIS_CONSTANT_BEM3D = 'c',
  /**
   * @brief Enum value to represent piecewise linear basis functions.
   */
  BASIS_LINEAR_BEM3D = 'l'
};

/**
 * This is just an abbreviation for the enum @ref _basisfunctionbem3d .
 */
typedef enum _basisfunctionbem3d basisfunctionbem3d;

/**
 * Defining a type for function that map from the boundary @f$ \Gamma @f$ of
 * the domain to a field @f$ \mathbb K @f$.
 *
 * This type is used to define functions that produce example dirichlet and
 * neumann data.
 *
 * @param x 3D-point the boundary @f$ \Gamma @f$.
 * @param n Corresponding outpointing normal vector to <tt>x</tt>.
 *
 * @return Returns evaluation of the function in <tt>x</tt> with normal vector
 * <tt>n</tt>.
 */
typedef field (*boundary_func3d)(const real *x, const real *n);

/**
 * This is just an abbreviation for the struct @ref _listnode .
 */
typedef struct _listnode listnode;

/**
 * Pointer to a @ref _listnode "listnode" object.
 */
typedef listnode *plistnode;

/**
 * This is just an abbreviation for the struct @ref _tri_list .
 */
typedef struct _tri_list tri_list;

/**
 * Pointer to a @ref _tri_list "tri_list" object.
 */
typedef tri_list *ptri_list;

/**
 * This is just an abbreviation for the struct @ref _vert_list .
 */
typedef struct _vert_list vert_list;

/**
 * Pointer to a @ref _vert_list "vert_list" object.
 */
typedef vert_list *pvert_list;

/**
 * @brief Main container object for computation of BEM related matrices and
 * vectors.
 *
 * This struct defines the essential part of all calculations concerning BEM.
 * Just creating a simple @ref _bem3d "bem3d" object is not intended, though
 * possible. Therefore call a problem specific constructor such as @ref
 * new_slp_laplace_bem3d or @ref new_dlp_laplace_bem3d . This will initialize all
 * needed callback functions to build up at least dense matrices.
 * To be able to create h- or h2matrix approximations one needs
 * to call one of the set_hmatrix_aprx* or set_h2matrix_aprx* depending on
 * the type of your matrix, to fully initialize
 * the @ref _bem3d "bem3d" object. Afterwards you can build up your desired
 * matrix approximation with a few simple functions calls, delivered by the @ref
 * _bem3d "bem3d" object, using your favored approximation technique.
 * Calling another of these \" set \" functions will reinitialize the @ref _bem3d
 * "bem3d" object and you can build another approximation with a different technique.
 */
struct _bem3d {
  /**
   * @brief Polyedric 3 dimensional surface mesh.
   *
   * Polyedric boundary mesh used for the BEM-problem.
   *  \see _surface3d.
   */
  pcsurface3d gr;

  /**
   * @brief Quadrature rules used within BEM computation.
   *
   * Singular quadrature formulas used within
   * boundary-integral-Operator computation.
   *
   * \see _singquad2d.
   */
  psingquad2d sq;

  /**
   * @brief Number of degrees of freedom for neumann data.
   */
  uint N_neumann;

  /**
   * @brief Type of basis function for neumann-data.
   */
  basisfunctionbem3d basis_neumann;

  /**
   * @brief Number of degrees of freedom for dirichlet data.
   */
  uint N_dirichlet;

  /**
   * @brief Type of basis function for dirichlet-data.
   */
  basisfunctionbem3d basis_dirichlet;

  /**
   * @brief Mass-matrix of reference basis functions.
   *
   * Mass-matrix for all combinations of reference basis functions
   * @f$ \widehat\varphi_i @f$ and @f$ \widehat\psi_j @f$ . Entries are
   * computed as: @f[ M_{ij} = \int_\Gamma \widehat\varphi_i(x) \cdot
   * \widehat\psi_j(x) \, \mathrm d x @f]
   */
  real *mass;

  /**
   * @brief Boundary integral operator + @f$\alpha@f$ mass matrix.
   */
  field alpha;

  /**
   * @brief This field describes the mapping from the vertices of a geometry to
   * the triangles they belong to.
   *
   * The length of this array is equal to the number of vertices. For each vertex
   * @f$ i @f$ the list <tt>v2t[i]</tt> enumerates all triangles that share this
   * vertex @f$ i @f$.<br>
   * This can be usefull within computation of matrix entries when linear
   * basis functions are involved.
   */
  plistnode *v2t;

  /**
   * @brief Computes nearfield entries of Galkerin matrices.
   *
   * This callback function computes
   *  'nearfield' entries of the underlying problem and integral-operator.
   *  @param ridx Defines the indices of row boundary elements used. If
   *  <tt>ridx</tt> equals NULL, then Elements <tt>0, ..., N->rows-1</tt> are used.
   *  @param cidx Defines the indices of column boundary elements used.
   *  If <tt>cidx</tt> equals NULL, then Elements <tt>0, ..., N->cols-1</tt> are
   *  used.
   *  @param bem Contains additional Information for the computation of the matrix
   *  entries.
   *  @param ntrans Is a boolean flag to indicates the way of storing the entries
   *  in matrix <tt>N</tt> . If <tt>ntrans == true</tt> the matrix entries will
   *  be stored in a transposed way.
   *  @param N For <tt>ntrans == false</tt> the matrix entries are computed as:
   *  @f[ N_{ij} = \int_\Gamma \int_\Gamma \varphi_i(x) \, g(x,y) \, \psi_j(y) \,
   *  \mathrm d y \, \mathrm d x \, ,@f]
   *  otherwise they are stored as
   *  @f[ N_{ji} = \int_\Gamma \int_\Gamma \varphi_i(x) \, g(x,y) \, \psi_j(y) \,
   * \mathrm d y \, \mathrm d x \, .@f]
   */
  void (*nearfield)(const uint *ridx, const uint *cidx, pcbem3d bem,
      bool ntrans, pamatrix N);

  /**
   * @brief Computes rank-k-approximations of a given block.
   *
   * This callback function computes a
   * rank-k-approximation of a matrix block given by row cluster <tt>rc</tt>
   * and column cluster <tt>cc</tt>. The Matrices <tt>A</tt> and <tt>B</tt>
   * are stored in a rank-k-matrix <tt>R</tt>.
   * @param rc Current row @ref _cluster "cluster" .
   * @param rname Enumerated number of current row @ref _cluster "cluster"
   * <tt>rc</tt>.
   * @param cc Current column @ref _cluster "cluster"
   * @param cname Enumerated number of current column @ref _cluster "cluster"
   * <tt>cc</tt>.
   * @param bem BEM-object containing additional information for computation
   * of the matrix entries.
   * @param R @ref _rkmatrix "Rank-k-matrix", which takes up the resulting matrices
   * <tt>A</tt>
   * and <tt>B</tt> . The rank <em>k</em> is defined by the underlying
   * approximation technique.
   */
  void (*farfield_rk)(pccluster rc, uint rname, pccluster cc, uint cname,
      pcbem3d bem, prkmatrix R);

  /**
   * @brief Computes coupling matrix @f$ S_b @f$ for @ref _uniform "uniform"
   * matrices.
   *
   * While using @ref _h2matrix
   * "h2-matrices" the coupling matrices @f$ S_b @f$
   * are computed by this callback function. Coupling matrices are stored in the
   * struct @ref _uniform "uniform" defining a certain block of the whole
   * @ref _h2matrix "h2-matrix".
   *
   * @param rname Enumerated number of current row @ref _cluster "cluster"
   * @ref _clusterbasis "clusterbasis" <tt>U->rb</tt>.
   * @param cname Enumerated number of current column @ref _clusterbasis
   * "clusterbasis" <tt>U->cb</tt>.
   * @param bem BEM-object containing additional information for computation
   * of the matrix entries.
   * @param U An Object of type @ref _uniform "uniform" containing the coupling
   * matrix @f$ S_b @f$ , the row @ref _clusterbasis "clusterbasis" <tt>U->rb</tt>
   * and the column @ref _clusterbasis "clusterbasis"  <tt>U->cb</tt> .
   */
  void (*farfield_u)(uint rname, uint cname, pcbem3d bem, puniform U);

  /**
   * @brief Computes the the matrix @f$ V_t @f$ for a leaf cluster @f$ t \in
   * \mathcal L_{\mathcal I} @f$ .
   *
   * @ref leaf_row is used for building up a @ref _clusterbasis "clusterbasis" for the
   * set @f$ \mathcal L_{\mathcal I} @f$ for
   * @ref _h2matrix "h2matrices".
   * For each @f$ t \in \mathcal L_{\mathcal I}
   * @f$ @ref leaf_row will construct the matrix @f$ V_t @f$ . In case of nested
   * @ref _clusterbasis "clusterbasis" @ref transfer_row has also to
   * be set to a valid function constructing suitable transfer matrices @f$ E_t
   * @f$ .
   * @param bem BEM-object containing additional information for computation
   * of the matrix entries.
   * @param rb current row @ref _clusterbasis "clusterbasis". The Matrix @f$ V_t
   * @f$ will be saved into <tt>rb->V</tt> .
   * @param rname Enumerated number of current row @ref _clusterbasis
   * "clusterbasis" <tt>rb</tt>.
   */
  void (*leaf_row)(pcbem3d bem, pclusterbasis rb, uint rname);

  /**
   * @brief Computes the the matrix @f$ W_s @f$ for a leaf cluster @f$ s \in
   * \mathcal L_{\mathcal J} @f$ .
   *
   * @ref leaf_col is used for building up a @ref _clusterbasis "clusterbasis" for the
   * set @f$ \mathcal L_{\mathcal J} @f$ for
   * @ref _h2matrix "h2matrices".
   * For each @f$ s \in \mathcal L_{\mathcal J}
   * @f$ @ref leaf_col will construct the matrix @f$ W_s @f$ . In case of nested
   * @ref _clusterbasis "clusterbasis" @ref transfer_col has also to
   * be set to a valid function constructing suitable transfer matrices @f$ F_s
   * @f$ .
   * @param bem BEM-object containing additional information for computation
   * of the matrix entries.
   * @param cb current column @ref _clusterbasis "clusterbasis". The Matrix @f$ W_s
   * @f$ will be saved into <tt>cb->V</tt> .
   * @param cname Enumerated number of current column @ref _clusterbasis
   * "clusterbasis" <tt>cb</tt>.
   */
  void (*leaf_col)(pcbem3d bem, pclusterbasis cb, uint cname);

  /**
   * @brief Computes the transfermatrices @f$ E_{t'} @f$ for a cluster @f$ t \in
   * \mathcal T_{\mathcal I} \, , t' \in \operatorname{sons}(t) @f$ .
   *
   * @ref transfer_row will build up the transfer matrices @f$ E_{t'} @f$ , for all
   * @f$ t \in \mathcal T_{\mathcal I}@f$ and @f$ t' \in \operatorname{sons}(t) @f$ .
   * For a cluster @f$ t \in \mathcal T_{\mathcal I} @f$ @ref transfer_row will compute
   * the matrices @f$ E_{t_1}, \ldots, E_{t_\tau} @f$ ,
   * @f$ \# \operatorname{sons}(t) = \tau @f$ . The matrix @f$ E_{t_1} @f$ is
   * stored in <tt>rb->son[0]->E</tt>, @f$ E_{t_2} @f$ is stored in
   * <tt>rb->son[1]->E</tt>, @f$ \ldots E_{t_\tau} @f$ is stored in
   * <tt>rb->son[@f$\tau-1@f$]->E</tt> .
   * @param bem BEM-object containing additional information for computation
   * of the matrix entries.
   * @param rb Current row @ref _clusterbasis "clusterbasis". The Matrices
   * @f$ E_{t'}, t' \in \operatorname{sons}(t) @f$ will be saved into
   * <tt>rb->son[t'-1]->E</tt> .
   * @param rname Enumerated number of current row @ref _clusterbasis
   * "clusterbasis" <tt>rb</tt>.
   */
  void (*transfer_row)(pcbem3d bem, pclusterbasis rb, uint rname);

  /**
   * @brief Computes the transfermatrices @f$ F_{s'} @f$ for a cluster @f$ s \in
   * \mathcal T_{\mathcal J} \, , s' \in \operatorname{sons}(s) @f$ .
   *
   * @ref transfer_col will build up the transfer matrices @f$ F_{s'} @f$ , for all
   * @f$ s \in \mathcal T_{\mathcal J}@f$ and @f$ s' \in \operatorname{sons}(s) @f$ .
   * For a cluster @f$ s \in \mathcal T_{\mathcal J} @f$ @ref transfer_col will compute
   * the matrices @f$ F_{s_1}, \ldots, F_{t_\sigma} @f$ ,
   * @f$ \# \operatorname{sons}(s) = \sigma @f$ . The matrix @f$ F_{s_1} @f$ is
   * stored in <tt>cb->son[0]->E</tt>, @f$ F_{s_2} @f$ is stored in
   * <tt>cb->son[1]->E</tt>, @f$ \ldots F_{t_\sigma} @f$ is stored in
   * <tt>cb->son[@f$\sigma-1@f$]->E</tt> .
   * @param bem BEM-object containing additional information for computation
   * of the matrix entries.
   * @param cb Current column @ref _clusterbasis "clusterbasis". The Matrices
   * @f$ F_{s'}, s' \in \operatorname{sons}(s) @f$ will be saved into
   * <tt>cb->son[s'-1]->E</tt> .
   * @param cname Enumerated number of current column @ref _clusterbasis
   * "clusterbasis" <tt>cb</tt>.
   */
  void (*transfer_col)(pcbem3d bem, pclusterbasis cb, uint cname);

  /**
   * @brief A collection of necessary data structures for approximating matrices
   * with various techniques.
   *
   * Necessary Objects for approximation matrices with different approximation
   * schemes are stored within @ref aprx.
   */
  paprxbem3d aprx;

  /**
   * @brief Some helpers to make parallel computations possible.
   *
   * For the sake of parallelism we need some auxiliary data structure, which are
   * all collected within the struct @ref par. Most commonly used is this struct
   * for enumerated non linear data structures such als cluster- or blocktrees.
   */
  pparbem3d par;

  /**
   * @brief A collection of callback functions for different types of kernel
   * evaluations.
   *
   * This struct defines a set of callback functions that are based on the fundamental
   * solution of the underlying problem. They are used to build up approximations
   * that will be used within @ref farfield_rk , @ref farfield_u , @ref leaf_row ,
   * @ref leaf_col , @ref transfer_row and @ref transfer_col.
   * In Order to receive correct result, the callback functions defined in @ref
   * kernels
   * have to be set correctly to the given problem by a constructor. These callback
   * functions also depend on the utilized basis functions for either neumann- and
   * dirichlet-data. As an example the constructors @ref new_slp_laplace_bem3d and
   * @ref new_dlp_laplace_bem3d should serve.
   * \see kernelbem2d
   */
  pkernelbem3d kernels;
};

/**
 * @brief Substructure containing callback functions to different types of
 * kernel evaluations.
 *
 * This struct consists of a set of callback function, that will evaluate the
 * kernel, its derivatives or integrals over the kernel and / or its derivatives.
 * Of course they depend on an appropriate problem to be solved.
 * The kernel function will be a function of the following form:
 * @f[
 * g : \mathbb R^3 \times \mathbb R^3 \to \mathbb K
 * @f]
 * In many application the field @f$ \mathbb K @f$ will just be the field of the
 * real number @f$ \mathbb R @f$ , as in the @ref laplacebem2d "laplace-problem".
 * But it can also be
 * the complex field @f$ \mathbb C @f$ as in the helmholtz-equation or a vector
 * valued function as in the lame-problem.
 */
struct _kernelbem3d {
  /**
   * @brief Evaluate the fundamental solution at given point sets <tt>X</tt> and
   * <tt>Y</tt>
   *
   * This callback will evaluate the fundamental solution @f$ g @f$ at points
   * @f$ x_i, y_j \in \mathbb R^3 @f$ and store the result into matrix V
   * at position @f$ V_{ij} @f$ .
   * @param X An array of 3D-vectors. <tt>X[i][0]</tt> will be the first
   * component of the i-th vector. Analogous <tt>X[i][1]</tt> will be the second
   * component of the i-th vector. The length of this array is determined by
   * <tt>V->rows</tt> .
   * @param Y An array of 3D-vectors. <tt>Y[i][0]</tt> will be the first
   * component of the i-th vector. Analogous <tt>Y[i][1]</tt> will be the second
   * component of the i-th vector. The length of this array is determined by
   * <tt>V->cols</tt> .
   * @param V V will contain the results of the kernel evaluations. It applies
   * @f[
   * V_{ij} = g(\vec x_i, \vec y_j) .
   * @f]
   */
  void (*fundamental)(const real (*X)[3], const real (*Y)[3], pamatrix V);

  /**
   * @brief Evaluate the normal derivative of the fundamental solution in respect
   * to the 2nd component at given point sets <tt>X</tt> and
   * <tt>Y</tt>
   *
   * This callback will evaluate the normal derivatives of the fundamental
   * solution @f$ g @f$ corresponding to @f$ y @f$ at points
   * @f$ x_i, y_j \in \mathbb R^3 @f$ and store the result into matrix V
   * at position @f$ V_{ij} @f$ .
   * @param X An array of 3D-vectors. <tt>X[i][0]</tt> will be the first
   * component of the i-th vector. Analogously <tt>X[i][1]</tt> will be the second
   * component of the i-th vector. The length of this array is determined by
   * <tt>V->rows</tt> .
   * @param Y An array of 3D-vectors. <tt>Y[i][0]</tt> will be the first
   * component of the i-th vector. Analogously <tt>Y[i][1]</tt> will be the second
   * component of the i-th vector. The length of this array is determined by
   * <tt>V->cols</tt> .
   * @param NY An array of normal vectors correspoding to the vectors defined
   * by Y and are stored in the same way as X and Y.
   * @param V V will contain the results of the kernel evaluations. It applies
   * @f[
   * V_{ij} = \frac{\partial g}{\partial n_y}(\vec x_i, \vec y_j) .
   * @f]
   */
  void (*dny_fundamental)(const real (*X)[3], const real (*Y)[3],
      const real (*NY)[3], pamatrix V);

  /**
   * @brief Evaluate the normal derivatives of the fundamental solution in respect
   * to the 1st and 2nd component at given point sets <tt>X</tt> and
   * <tt>Y</tt>
   *
   * This callback will evaluate the normal derivatives of the fundamental
   * solution @f$ g @f$ corresponding to @f$ x @f$ and @f$ y @f$ at points
   * @f$ x_i, y_j \in \mathbb R^3 @f$ and store the result into matrix V
   * at position @f$ V_{ij} @f$ .
   * @param X An array of 3D-vectors. <tt>X[i][0]</tt> will be the first
   * component of the i-th vector. Analogously <tt>X[i][1]</tt> will be the second
   * component of the i-th vector. The length of this array is determined by
   * <tt>V->rows</tt> .
   * @param NX An array of normal vectors correspoding to the vectors defined
   * by X and are stored in the same way as X and Y.
   * @param Y An array of 3D-vectors. <tt>Y[i][0]</tt> will be the first
   * component of the i-th vector. Analogously <tt>Y[i][1]</tt> will be the second
   * component of the i-th vector. The length of this array is determined by
   * <tt>V->cols</tt> .
   * @param NY An array of normal vectors correspoding to the vectors defined
   * by Y and are stored in the same way as X and Y.
   * @param V V will contain the results of the kernel evaluations. It applies
   * @f[
   * V_{ij} = \frac{\partial^2 g}{\partial n_x \partial n_y}(\vec x_i, \vec y_j) .
   * @f]
   */
  void (*dnx_dny_fundamental)(const real (*X)[3], const real (*NX)[3],
      const real (*Y)[3], const real (*NY)[3], pamatrix V);

  /**
   * @brief Integrate the kernel function within the 1st component.
   *
   * This callback will evaluate the integral
   * @f[
   * \int_\Gamma \varphi(\vec x) \, \gamma(\vec x, \vec z) \, \mathrm d \vec x
   * @f]
   * with @f$ \gamma @f$ being a kernel function depending on a specific integral
   * operator. In case of the single layer potential it applies @f$ \gamma = g @f$ .
   * But in case of the double layer potential e.g. it applies @f$ \gamma =
   * \frac{\partial g}{\partial n_y} @f$ .
   * @param idx This array describes the permutation of the degrees of freedom.
   * Its length is determined by <tt>A->rows</tt> . In case <tt>idx == NULL</tt>
   * it is assumed the degrees of freedom are @f$ 0, 1, \ldots , \texttt{A->rows}
   *  -1 @f$
   * instead.
   * @param Z An array of 3D-vectors. <tt>Z[i][0]</tt> will be the first
   * component of the i-th vector. Analogously <tt>Z[i][1]</tt> will be the second
   * component of the i-th vector. The length of this array is determined by
   * <tt>A->cols</tt> .
   * @param bem BEM-object containing additional information for computation
   of the matrix entries.
   * @param A A will contain the results of the integral evaluations. It applies
   * @f[
   * A_{ij} = \int_\Gamma \varphi_i(\vec x) \, \gamma(\vec x, \vec z_j) \, \mathrm d \vec x .
   * @f]
   */
  void (*kernel_row)(const uint *idx, const real (*Z)[3], pcbem3d bem,
      pamatrix A);

  /**
   * @brief Integrate the kernel function within the 2nd component.
   *
   * This callback will evaluate the integral
   * @f[
   * \int_\Gamma \gamma(\vec z, \vec y) \, \psi(\vec y) \, \mathrm d \vec y
   * @f]
   * with @f$ \gamma @f$ being a kernel function depending on a specific integral
   * operator. In case of the single layer potential it applies @f$ \gamma = g @f$ .
   * But in case of the double layer potential e.g. it applies @f$ \gamma =
   * \frac{\partial g}{\partial n_y} @f$ .
   * @param idx This array describes the permutation of the degrees of freedom.
   * Its length is determined by <tt>B->rows</tt> . In case <tt>idx == NULL</tt>
   * it is assumed the degrees of freedom are @f$ 0, 1, \ldots , \texttt{B->rows}
   *  -1 @f$
   * instead.
   * @param Z An array of 3D-vectors. <tt>Z[i][0]</tt> will be the first
   * component of the i-th vector. Analogously <tt>Z[i][1]</tt> will be the second
   * component of the i-th vector. The length of this array is determined by
   * <tt>B->cols</tt> .
   * @param bem BEM-object containing additional information for computation
   of the matrix entries.
   * @param B B will contain the results of the integral evaluations. It applies
   * @f[
   * B_{ij} = \int_\Gamma \gamma(\vec z_j, \vec y) \,
   * \psi_i(\vec y) \, \mathrm d \vec y .
   * @f]
   */
  void (*kernel_col)(const uint *idx, const real (*Z)[3], pcbem3d bem,
      pamatrix B);

  /**
   * @brief Integrate the normal derivative of the kernel function with respect
   * to the 2nd component within the 1st component.
   *
   * This callback will evaluate the integral
   * @f[
   * \int_\Gamma \varphi(\vec x) \, \frac{\partial \gamma}{\partial n_z}
   * (\vec x, \vec z) \, \mathrm d \vec x
   * @f]
   * with @f$ \gamma @f$ being a kernel function depending on a specific integral
   * operator. In case of the single layer potential it applies @f$ \gamma = g @f$ .
   * But in case of the double layer potential e.g. it applies @f$ \gamma =
   * \frac{\partial g}{\partial n_y} @f$ .
   * @param idx This array describes the permutation of the degrees of freedom.
   * Its length is determined by <tt>A->rows</tt> . In case <tt>idx == NULL</tt>
   * it is assumed the degrees of freedom are @f$ 0, 1, \ldots , \texttt{A->rows}
   *  -1 @f$
   * instead.
   * @param Z An array of 3D-vectors. <tt>Z[i][0]</tt> will be the first
   * component of the i-th vector. Analogously <tt>Z[i][1]</tt> will be the second
   * component of the i-th vector. The length of this array is determined by
   * <tt>A->cols</tt> .
   * @param NZ An array Normal vectors corresponding to the vectors Z.
   * @param bem BEM-object containing additional information for computation
   of the matrix entries.
   * @param A A will contain the results of the integral evaluations. It applies
   * @f[
   * A_{ij} = \int_\Gamma \varphi_i(\vec x) \, \frac{\partial \gamma}{\partial n_z}
   * (\vec x, \vec z_j) \, \mathrm d \vec x .
   * @f]
   */
  void (*dnz_kernel_row)(const uint *idx, const real (*Z)[3],
      const real (*NZ)[3], pcbem3d bem, pamatrix A);

  /**
   * @brief Integrate the normal derivative of the kernel function with respect
   * to the 1st component within the 2nd component.
   *
   * This callback will evaluate the integral
   * @f[
   * \int_\Gamma \frac{\partial \gamma}{\partial n_z}
   * (\vec z, \vec y) \, \psi(\vec y) \, \mathrm d \vec y
   * @f]
   * with @f$ \gamma @f$ being a kernel function depending on a specific integral
   * operator. In case of the single layer potential it applies @f$ \gamma = g @f$ .
   * But in case of the double layer potential e.g. it applies @f$ \gamma =
   * \frac{\partial g}{\partial n_y} @f$ .
   * @param idx This array describes the permutation of the degrees of freedom.
   * Its length is determined by <tt>B->rows</tt> . In case <tt>idx == NULL</tt>
   * it is assumed the degrees of freedom are @f$ 0, 1, \ldots , \texttt{B->rows}
   *  -1 @f$
   * instead.
   * @param Z An array of 3D-vectors. <tt>Z[i][0]</tt> will be the first
   * component of the i-th vector. Analogously <tt>Z[i][1]</tt> will be the second
   * component of the i-th vector. The length of this array is determined by
   * <tt>B->cols</tt> .
   * @param NZ An array Normal vectors corresponding to the vectors Z.
   * @param bem BEM-object containing additional information for computation
   of the matrix entries.
   * @param B B will contain the results of the integral evaluations. It applies
   * @f[
   * B_{ij} = \int_\Gamma \frac{\partial \gamma}{\partial n_z}
   * (\vec z_j, \vec y) \, \psi_i(\vec y) \, \mathrm d \vec y .
   * @f]
   */
  void (*dnz_kernel_col)(const uint *idx, const real (*Z)[3],
      const real (*NZ)[3], pcbem3d bem, pamatrix B);

  /**
   * @brief Integrate the fundamental solution within the 1st component.
   *
   * This callback will evaluate the integral
   * @f[
   * \int_\Gamma \varphi(\vec x) \, g(\vec x, \vec z) \, \mathrm d \vec x
   * @f]
   * @param idx This array describes the permutation of the degrees of freedom.
   * Its length is determined by <tt>A->rows</tt> . In case <tt>idx == NULL</tt>
   * it is assumed the degrees of freedom are @f$ 0, 1, \ldots , \texttt{A->rows}
   *  -1 @f$
   * instead.
   * @param Z An array of 3D-vectors. <tt>Z[i][0]</tt> will be the first
   * component of the i-th vector. Analogously <tt>Z[i][1]</tt> will be the second
   * component of the i-th vector. The length of this array is determined by
   * <tt>A->cols</tt> .
   * @param bem BEM-object containing additional information for computation
   of the matrix entries.
   * @param A A will contain the results of the integral evaluations. It applies
   * @f[
   * A_{ij} = \int_\Gamma \varphi_i(\vec x) \, g(\vec x, \vec z_j) \, \mathrm d \vec x .
   * @f]
   */
  void (*fundamental_row)(const uint *idx, const real (*Z)[3], pcbem3d bem,
      pamatrix A);

  /**
   * @brief Integrate the fundamental solution within the 2nd component.
   *
   * This callback will evaluate the integral
   * @f[
   * \int_\Gamma g(\vec z, \vec y) \, \psi(\vec y) \, \mathrm d \vec y
   * @f]
   * @param idx This array describes the permutation of the degrees of freedom.
   * Its length is determined by <tt>B->rows</tt> . In case <tt>idx == NULL</tt>
   * it is assumed the degrees of freedom are @f$ 0, 1, \ldots , \texttt{B->rows}
   *  -1 @f$
   * instead.
   * @param Z An array of 3D-vectors. <tt>Z[i][0]</tt> will be the first
   * component of the i-th vector. Analogously <tt>Z[i][1]</tt> will be the second
   * component of the i-th vector. The length of this array is determined by
   * <tt>B->cols</tt> .
   * @param bem BEM-object containing additional information for computation
   of the matrix entries.
   * @param B B will contain the results of the integral evaluations. It applies
   * @f[
   * B_{ij} = \int_\Gamma g(\vec z_j, \vec y) \,
   * \psi_i(\vec y) \, \mathrm d \vec y .
   * @f]
   */
  void (*fundamental_col)(const uint *idx, const real (*Z)[3], pcbem3d bem,
      pamatrix B);

  /**
   * @brief Integrate the normal derivative of the fundamental solution with respect
   * to the 2nd component within the 1st component.
   *
   * This callback will evaluate the integral
   * @f[
   * \int_\Gamma \varphi(\vec x) \, \frac{\partial g}{\partial n_z}
   * (\vec x, \vec z) \, \mathrm d \vec x
   * @f]
   * @param idx This array describes the permutation of the degrees of freedom.
   * Its length is determined by <tt>A->rows</tt> . In case <tt>idx == NULL</tt>
   * it is assumed the degrees of freedom are @f$ 0, 1, \ldots , \texttt{A->rows}
   *  -1 @f$
   * instead.
   * @param Z An array of 3D-vectors. <tt>Z[i][0]</tt> will be the first
   * component of the i-th vector. Analogously <tt>Z[i][1]</tt> will be the second
   * component of the i-th vector. The length of this array is determined by
   * <tt>A->cols</tt> .
   * @param NZ An array Normal vectors corresponding to the vectors Z.
   * @param bem BEM-object containing additional information for computation
   of the matrix entries.
   * @param A A will contain the results of the integral evaluations. It applies
   * @f[
   * A_{ij} = \int_\Gamma \varphi_i(\vec x) \, \frac{\partial g}{\partial n_z}
   * (\vec x, \vec z_j) \, \mathrm d \vec x .
   * @f]
   */
  void (*dnz_fundamental_row)(const uint *idx, const real (*Z)[3],
      const real (*NZ)[3], pcbem3d bem, pamatrix A);

  /**
   * @brief Integrate the normal derivative of the fundamental solution with respect
   * to the 1st component within the 2nd component.
   *
   * This callback will evaluate the integral
   * @f[
   * \int_\Gamma \frac{\partial g}{\partial n_z}
   * (\vec z, \vec y) \, \psi(\vec y) \, \mathrm d \vec y
   * @f]
   * @param idx This array describes the permutation of the degrees of freedom.
   * Its length is determined by <tt>B->rows</tt> . In case <tt>idx == NULL</tt>
   * it is assumed the degrees of freedom are @f$ 0, 1, \ldots , \texttt{B->rows}
   *  -1 @f$
   * instead.
   * @param Z An array of 3D-vectors. <tt>Z[i][0]</tt> will be the first
   * component of the i-th vector. Analogously <tt>Z[i][1]</tt> will be the second
   * component of the i-th vector. The length of this array is determined by
   * <tt>B->cols</tt> .
   * @param NZ An array Normal vectors corresponding to the vectors Z.
   * @param bem BEM-object containing additional information for computation
   of the matrix entries.
   * @param B B will contain the results of the integral evaluations. It applies
   * @f[
   * B_{ij} = \int_\Gamma \frac{\partial g}{\partial n_z}
   * (\vec z_j, \vec y) \, \psi_i(\vec y) \, \mathrm d \vec y .
   * @f]
   */
  void (*dnz_fundamental_col)(const uint *idx, const real (*Z)[3],
      const real (*NZ)[3], pcbem3d bem, pamatrix B);

  /**
   * @brief Integrate the Lagrange polynomials or their derivatives within the
   * 1st component.
   *
   * This callback will evaluate the integral
   * @f[
   * \int_\Gamma \varphi(\vec x) \, \mathcal L(\vec x) \, \mathrm d \vec x
   * @f]
   * with @f$ \mathcal L @f$ being a Lagrange polynomial or its derivative.
   * @param idx This array describes the permutation of the degrees of freedom.
   * Its length is determined by <tt>V->rows</tt> . In case <tt>idx == NULL</tt>
   * it is assumed the degrees of freedom are @f$ 0, 1, \ldots , \texttt{V->rows}
   *  -1 @f$
   * instead.
   * @param px A Vector of type @ref _avector "avector" that contains interpolation
   * points in x-direction.
   * @param py A Vector of type @ref _avector "avector" that contains interpolation
   * points in y-direction.
   * @param pz A Vector of type @ref _avector "avector" that contains interpolation
   * points in z-direction.
   * @param bem BEM-object containing additional information for computation
   of the matrix entries.
   * @param V V will contain the results of the integral evaluations. It applies
   * @f[
   * V_{ij} = \int_\Gamma \varphi_i(\vec x) \, \mathcal L_j(\vec x) \,
   * \mathrm d \vec x .
   * @f]
   * The index @f$ j @f$ is computed in a tensor way. Having @f$ m_x @f$ Points
   * in x-direction, @f$ m_y @f$ Points in y-direction and @f$ m_y @f$ Points in
   * z-direction, then using the @f$
   * j_x @f$ -th, the @f$ j_y @f$ -th and the @f$ j_z @f$ -th point will result
   * in matrix index
   * @f$ j = j_x + j_y \cdot m_x  + j_z \cdot m_x \cdot m_y@f$ .
   */
  void (*lagrange_row)(const uint *idx, pcavector px, pcavector py,
      pcavector pz, pcbem3d bem, pamatrix V);

  /**
   * @brief Integrate the Lagrange polynomials or their derivatives within the
   * 2nd component.
   *
   * This callback will evaluate the integral
   * @f[
   * \int_\Gamma  \mathcal L(\vec y) \, \psi(\vec y) \, \mathrm d \vec y
   * @f]
   * with @f$ \mathcal L @f$ being a Lagrange polynomial or its derivative.
   * @param idx This array describes the permutation of the degrees of freedom.
   * Its length is determined by <tt>W->rows</tt> . In case <tt>idx == NULL</tt>
   * it is assumed the degrees of freedom are @f$ 0, 1, \ldots , \texttt{W->rows}
   *  -1 @f$
   * instead.
   * @param px A Vector of type @ref _avector "avector" that contains interpolation
   * points in x-direction.
   * @param py A Vector of type @ref _avector "avector" that contains interpolation
   * points in y-direction.
   * @param pz A Vector of type @ref _avector "avector" that contains interpolation
   * points in z-direction.
   * @param bem BEM-object containing additional information for computation
   of the matrix entries.
   * @param W W will contain the results of the integral evaluations. It applies
   * @f[
   * W_{ij} = \int_\Gamma \mathcal L_j(\vec y) \, \psi_i(\vec y) \,
   * \mathrm d \vec y .
   * @f]
   * The index @f$ j @f$ is computed in a tensor way. Having @f$ m_x @f$ Points
   * in x-direction, @f$ m_y @f$ Points in y-direction and @f$ m_y @f$ Points in
   * z-direction, then using the @f$
   * j_x @f$ -th, the @f$ j_y @f$ -th and the @f$ j_z @f$ -th point will result
   * in matrix index
   * @f$ j = j_x + j_y \cdot m_x  + j_z \cdot m_x \cdot m_y@f$ .
   */
  void (*lagrange_col)(const uint *idx, pcavector px, pcavector py,
      pcavector pz, pcbem3d bem, pamatrix W);
};

/**
 * @brief simple singly linked list to store unsigned inter values.
 *
 * This Struct is used to build up a vertex-to-triangle
 * (@ref setup_vertex_to_triangle_map_bem3d) needed by piecewise
 * linear basis function in @ref bem3d.
 */
struct _listnode {
  /** @brief Dataelement to be stored. here the number of a triangle. */
  uint data;
  /** @brief Pointer to the next @ref _listnode element. */
  plistnode next;
};

/**
 * @brief Simple singly connected list to efficiently store a list of triangles.
 *
 * This struct is needed to efficiently compute integrals while using piecewise
 * linear basis function. It is used to build up a list of triangle that are
 * connected to vertex basis functions
 */
struct _tri_list {
  /** @brief Index of a the triangle to be stored. */
  uint t;
  /**
   * @brief a List of vertices whose basis functions are connected with this
   * triangle and are needed within the computation of integrals.
   */
  pvert_list vl;
  /** @brief Pointer to the next @ref _tri_list element. */
  ptri_list next;
};

/**
 * @brief Simple singly connected list to efficiently store a list of vertices.
 *
 * This struct is needed to efficiently compute integrals while using piecewise
 * linear basis function. It is used to build up a list of vertex basis functions
 * that are connected to a triangle and needed within the computations.
 */
struct _vert_list {
  /** @brief Index of the vertex to be stored. */
  uint v;
  /** @brief Pointer to the next @ref _vert_list element. */
  pvert_list next;
};

/* ------------------------------------------------------------
 Constructors and destructors
 ------------------------------------------------------------ */

/**
 * @brief Create a new list to store a number of vertex indices.
 *
 * New list elements will be appended to the head of the list.
 * Therefore the argument <tt>next</tt> has to point to the old list.
 *
 * @param next Pointer to successor.
 * @return A new list with successor <tt>next</tt> is returned.
 */
HEADER_PREFIX
pvert_list new_vert_list(pvert_list next);

/**
 * @brief Recursively deletes a @ref vert_list.
 *
 * @param vl @ref vert_list to be deleted.
 */
HEADER_PREFIX
void del_vert_list(pvert_list vl);

/**
 * @brief Create a new list to store a number of triangles indices.
 *
 * New list elements will be appended to the head of the list.
 * Therefore the argument <tt>next</tt> has to point to the old list.
 *
 * @param next Pointer to successor.
 * @return A new list with successor <tt>next</tt> is returned.
 */
HEADER_PREFIX
ptri_list new_tri_list(ptri_list next);

/**
 * @brief Recursively deletes a @ref tri_list.
 *
 * @param tl @ref tri_list to be deleted.
 */
HEADER_PREFIX
void del_tri_list(ptri_list tl);

/**
 * @brief Main constructor for @ref _bem3d "bem3d" objects.
 *
 * Primary constructor for @ref _bem3d "bem3d" objects. Although a valid
 * @ref _bem3d "bem3d" object will be returned it is not intended to use this
 * constructor. Instead use a problem specific constructor such as @ref
 * new_slp_laplace_bem3d or @ref new_dlp_laplace_bem3d .
 * @param gr 3D geometry described as polyedric surface mesh.
 * @return returns a valid @ref _bem3d "bem3d" object that will be used for
 * almost all computations concerning a BEM-application.
 */
HEADER_PREFIX pbem3d new_bem3d(pcsurface3d gr);

/**
 * @brief Destructor for @ref _bem3d "bem3d" objects.
 *
 * Delete a @ref _bem3d "bem3d" object.
 * @param bem @ref _bem3d "bem3d" object to be deleted.
 */
HEADER_PREFIX void del_bem3d(pbem3d bem);

/* ------------------------------------------------------------
 Methods to build clustertrees
 ------------------------------------------------------------ */

/**
 * @brief Creates a @ref _clustergeometry "clustergeometry" object for a BEM-Problem
 * using piecewise constant basis functions.
 *
 * The geometry is taken from the @ref _bem3d "bem" object as <tt>bem->gr</tt> .
 *
 * @param bem At this stage @ref _bem3d "bem" just serves as a container for
 * the geometry <tt>bem->gr</tt> itself.
 * @param idx This method will allocate enough memory into idx to
 * enumerate each degree of freedom. After calling @ref build_bem3d_const_clustergeometry
 * the value of idx will be 0, 1, ... N-1 if N is the number of degrees of
 * freedom for piecewise constant basis functions, which will be equal to the
 * number of triangles in the geometry.
 * @return Function will return a valid @ref _clustergeometry "clustergeometry"
 * object that can be used to construct a clustertree along with the array
 * of degrees of freedom idx.
 */
HEADER_PREFIX pclustergeometry build_bem3d_const_clustergeometry(pcbem3d bem,
    uint **idx);

/**
 * @brief Creates a @ref _clustergeometry "clustergeometry" object for a BEM-Problem
 * using linear basis functions.
 *
 * The geometry is taken from the @ref _bem3d "bem" object as <tt>bem->gr</tt> .
 *
 * @param bem At this stage @ref _bem3d "bem" just serves as a container for
 * the geometry <tt>bem->gr</tt> itself.
 * @param idx This method will allocate enough memory into idx to
 * enumerate each degree of freedom. After calling @ref build_bem3d_linear_clustergeometry
 * the value of idx will be 0, 1, ... N-1 if N is the number of degrees of
 * freedom for linear basis functions, which will be equal to the
 * number of vertices in the geometry.
 *
 * @return Function will return a valid @ref _clustergeometry "clustergeometry"
 * object that can be used to construct a clustertree along with the array
 * of degrees of freedom idx.
 */
HEADER_PREFIX pclustergeometry build_bem3d_linear_clustergeometry(pcbem3d bem,
    uint **idx);

/**
 * @brief Creates a @ref _clustergeometry "clustergeometry" object for a BEM-Problem
 * using the type of basis functions specified by basis.
 *
 * The geometry is taken from the @ref _bem3d "bem" object as <tt>bem->gr</tt> .
 *
 * @param bem At this stage @ref _bem3d "bem" just serves as a container for
 * the geometry <tt>bem->gr</tt> itself.
 * @param idx This method will allocate enough memory into idx to
 * enumerate each degree of freedom. After calling @ref build_bem3d_clustergeometry
 * the value of idx will be 0, 1, ... N-1 if N is the number of degrees of
 * freedom for basis functions defined by basis.
 * @param basis This defines the type of basis functions that should be used to
 * construct the @ref _clustergeometry "clustergeometry" object. According to the
 * type @ref _basisfunctionbem3d "basisfunctionbem3d" there are actual just to
 * valid valued for basis: Either @ref BASIS_CONSTANT_BEM3D or
 * @ref BASIS_LINEAR_BEM3D. Depending on that value either @ref
 * build_bem3d_const_clustergeometry or @ref build_bem3d_linear_clustergeometry will be
 * called to build the object.
 * @return Function will return a valid @ref _clustergeometry "clustergeometry"
 * object that can be used to construct a clustertree along with the array
 * of degrees of freedom idx.
 */
HEADER_PREFIX pclustergeometry build_bem3d_clustergeometry(pcbem3d bem,
    uint **idx, basisfunctionbem3d basis);

/**
 * @brief Creates a @ref _cluster "clustertree" for specified basis functions.
 *
 * This function will build up a clustertree for a BEM-problem using basis
 * functions of type basis. The maximal size of the leaves of the resulting
 * tree are limited by clf. At first this function will call @ref
 * build_bem3d_clustergeometry and uses the temporary result to contruct a
 * clustertree using @ref build_adaptive_cluster.
 *@param bem At this stage @ref _bem3d "bem" just serves as a container for
 * the geometry <tt>bem->gr</tt> itself.
 *@param clf This parameter limits the maximals size of leafclusters. It holds:
 * @f[ \# t \leq \texttt{clf} , \, \text{for all } t \in \mathcal L @f]
 *@param basis This defines the type of basis functions that should be used to
 * construct the @ref _cluster "cluster" object. According to the
 * type @ref _basisfunctionbem3d "basisfunctionbem3d" there are actual just to
 * valid valued for basis: Either @ref BASIS_CONSTANT_BEM3D or
 * @ref BASIS_LINEAR_BEM3D.
 *@return A suitable clustertree for basis functions defined by basis and
 * using @ref build_adaptive_cluster to build up the tree.
 */
HEADER_PREFIX pcluster build_bem3d_cluster(pcbem3d bem, uint clf,
    basisfunctionbem3d basis);

/* ------------------------------------------------------------
 Initializerfunctions for h-matrix approximations
 ------------------------------------------------------------ */

/**
 * @brief Initialize the @ref _bem3d "bem" object for on the fly recompression
 * techniques.
 *
 * @param bem According to the other parameters within <tt>bem->aprx</tt> there
 * are set some flags that indicate which of the recompression techniques should
 * be used while building up a @ref _hmatrix "hmatrix" .
 * @param recomp If this flag is set to <tt>true</tt> a blockwise recompression
 * will be applied. For each block @f$ b = (t,s) \in \mathcal L^+ @f$ independent on
 * the approximation technique we will yield an approximation of the following
 * kind:
 * @f[
 * G_{|t \times s} \approx A B^* \quad A \in \mathbb K^{\hat t \times k} \,
 *  B \in \mathbb K^{\hat s \times k}
 * @f]
 * After compression we yield a new rank-k-approximation holding
 * @f[
 * G_{|t \times s} \approx \widetilde A \widetilde B^* \quad \widetilde A \in
 * \mathbb K^{\hat t \times \widetilde k} \,  \widetilde B \in
 * \mathbb K^{\hat s \times \widetilde k}
 * @f]
 * and @f$ \widetilde k \leq k @f$ depending on the accuracy accur_recomp
 * @param accur_recomp The accuracy the blockwise recompression will used to
 * compress a block with SVD.
 * @param coarsen If a block consists of @f$ \tau @f$ sons which are all leafs
 * of the blocktree, then it can be more efficient to store them as a single
 * admissible leaf. This is done by \"coarsening\" recursively using the SVD.
 * @param accur_coarsen The accuracy the coarsen technique will use to determine
 * the coarsened block structure.
 */
HEADER_PREFIX void setup_hmatrix_recomp_bem3d(pbem3d bem, bool recomp,
    real accur_recomp, bool coarsen, real accur_coarsen);

/* ------------------------------------------------------------
 Interpolation
 ------------------------------------------------------------ */

/**
 * @brief This function initializes the @ref _bem3d "bem3d" object for approximating
 * a matrix with tensorinterpolation within the row @ref _cluster "cluster".
 *
 * The approximation of an admissible block @f$ b = (t,s) \in
 * \mathcal L_{\mathcal I \times \mathcal J}^+ @f$ is done by:
 * @f[
 * G_{|t \times s} \approx A_t \, B_{t,s}^* \quad \text{with}
 * @f]
 * @f[
 * \left( A_t \right)_{i\mu} := \int_\Gamma \, \varphi_i(\vec x) \,
 * \mathcal L_\mu(\vec x) \, \mathrm d \vec x \quad \text{and}
 * @f]
 * @f[
 * \left( B_{t,s} \right)_{j\mu} := \int_\Gamma \, \gamma(\vec \xi_\mu , \, \vec y
 * ) \, \psi_j(\vec y) \, \mathrm d \vec y \, .
 * @f]
 * with @f$ \gamma @f$ being the kernel function of the underlying integral operator.
 * In case of the single layer potential this equal to the fundamental solution
 * @f$ g @f$ , in case of double layer potential this applies to @f$ \gamma =
 * \frac{\partial g}{\partial n_y} @f$ .
 *
 * @param bem All needed callback functions and parameters for this approximation
 * scheme are set within the bem object.
 * @param rc Root of the row @ref _cluster "clustertree".
 * @param cc Root of the column @ref _cluster "clustertree".
 * @param tree Root of the @ref _block "blocktree".
 * @param m Number of Chebyshev interpolation points in each spatial dimension.
 */
HEADER_PREFIX void setup_hmatrix_aprx_inter_row_bem3d(pbem3d bem, pccluster rc,
    pccluster cc, pcblock tree, uint m);

/**
 * @brief This function initializes the @ref _bem3d "bem3d" object for approximating
 * a matrix with tensorinterpolation within the column @ref _cluster "cluster".
 *
 * The approximation of an admissible block @f$ b = (t,s) \in
 * \mathcal L_{\mathcal I \times \mathcal J}^+ @f$ is done by:
 * @f[
 * G_{|t \times s} \approx A_{t,s} \, B_s^* \quad \text{with}
 * @f]
 * @f[
 * \left( A_{t,s} \right)_{i\mu} := \int_\Gamma \, \varphi_i(\vec x) \,
 * g(\vec x , \, \vec \xi_\mu) \, \mathrm d \vec x \quad \text{and}
 * @f]
 * @f[
 * \left( B_s \right)_{j\mu} := \int_\Gamma \, \Lambda_\mu(\vec y) \,
 * \psi_j(\vec y) \, \mathrm d \vec y \, .
 * @f]
 * with @f$ \Lambda @f$ being either the Lagrange polynomial @f$ \mathcal L @f$
 * itself in case of single layer potential, or @f$ \Lambda =
 * \frac{\partial \mathcal L}{\partial n} @f$ in case of double layer
 * potential.
 *
 * @param bem All needed callback functions and parameters for this approximation
 * scheme are set within the bem object.
 * @param rc Root of the row @ref _cluster "clustertree".
 * @param cc Root of the column @ref _cluster "clustertree".
 * @param tree Root of the @ref _block "blocktree".
 * @param m Number of Chebyshev interpolation points in each spatial dimension.
 */
HEADER_PREFIX void setup_hmatrix_aprx_inter_col_bem3d(pbem3d bem, pccluster rc,
    pccluster cc, pcblock tree, uint m);

/**
 * @brief This function initializes the @ref _bem3d "bem3d" object for approximating
 * a matrix with tensorinterpolation within one of row or column @ref _cluster
 * "cluster".
 *
 * The interpolation depends on the condition:
 * @f[
 * G_{|t \times s} \approx \begin{cases}\begin{array}{ll}
 * A_t \, B_{t,s}^* &: \operatorname{diam}(t) <\operatorname{diam}(s)\\
 * A_{t,s} \, B_s^* &: \operatorname{diam}(t) \geq \operatorname{diam}(s)
 * \end{array}
 * \end{cases}
 * @f]
 * For definition of these matrices see @ref setup_hmatrix_aprx_inter_row_bem3d and
 * @ref setup_hmatrix_aprx_inter_col_bem3d .
 *
 * @param bem All needed callback functions and parameters for this approximation
 * scheme are set within the bem object.
 * @param rc Root of the row @ref _cluster "clustertree".
 * @param cc Root of the column @ref _cluster "clustertree".
 * @param tree Root of the @ref _block "blocktree".
 * @param m Number of Chebyshev interpolation points in each spatial dimension.
 */
HEADER_PREFIX void setup_hmatrix_aprx_inter_mixed_bem3d(pbem3d bem,
    pccluster rc, pccluster cc, pcblock tree, uint m);

/* ------------------------------------------------------------
 Green
 ------------------------------------------------------------ */
/**
 * @brief creating hmatrix approximation using green's method with row
 * @ref _cluster "cluster" .
 *
 * This function initializes the @ref _bem3d "bem2d" object for approximating
 * a matrix using green's formula. The approximation is based upon an expansion
 * around the row @ref _cluster "cluster".
 * For an admissible block @f$ b = (t,s) \in \mathcal L_{\mathcal I
 * \times \mathcal J}^+ @f$ one yields an approximation in shape of
 * @f[
 * G_{|t \times s} \approx A_t \, B_{t,s}^* \quad \text{with}
 * @f]
 * @f[
 * \left( A_t^1 \right)_{i\nu} := \int_\Gamma \, \varphi_i(\vec x) \, g(\vec x,
 * \vec z_\nu) \, \mathrm d \vec x \quad ,
 * @f]
 * @f[
 * \left( A_t^2 \right)_{i\nu} := \omega_\nu \, \int_\Gamma \, \varphi_i(\vec x) \,
 * \frac{\partial g}{\partial n_z} (\vec x, \vec z_\nu) \, \mathrm d \vec x \quad ,
 * @f]
 * @f[
 * A_t = \begin{pmatrix} A^1_t & A^2_t \end{pmatrix} \quad ,
 * @f]
 *
 * @f[
 * \left( B_{t,s}^1 \right)_{j\nu} := \omega_\nu \, \int_\Gamma  \,
 * \frac{\partial \gamma}{\partial n_z} (\vec z_\nu,
 * \vec y) \, \psi_j(\vec y) \, \mathrm d \vec y \quad ,
 * @f]
 * @f[
 * \left( B_{t,s}^2 \right)_{j\nu} := -\int_\Gamma  \, \gamma(\vec z_\nu,
 * \vec y) \, \psi_j(\vec y) \, \mathrm d \vec y \quad \text{and}
 * @f]
 * @f[
 * B_{t,s} = \begin{pmatrix} B_{t,s}^1 & B_{t,s}^2 \end{pmatrix} .
 * @f]
 * @f$ \gamma @f$ is the underlying kernel function defined by the the integral
 * operator. In case of single layer potential it applies @f$ \gamma = g @f$ but
 * in case of double layer potential it applies @f$ \gamma =
 * \frac{\partial g}{\partial n_y} @f$ .
 * The green quadrature points @f$ z_\nu @f$ and the weight @f$ \omega_\nu @f$
 * depend on the choice of <tt>quadpoints</tt> and <tt>delta</tt>, which define
 * the parameterization
 * around the row \ref _cluster "cluster".
 *
 * @param bem All needed callback functions and parameters for this approximation
 * scheme are set within the bem object.
 * @param rc Root of the row @ref _cluster "clustertree".
 * @param cc Root of the column @ref _cluster "clustertree".
 * @param tree Root of the @ref _block "blocktree".
 * @param m Number of gaussian quadrature points in each patch of the
 * parameterization.
 * @param l Number of subdivisions for gaussian quadrature in each patch of the
 * parameterization.
 * @param delta Defines the distance between the bounding box @f$ B_t @f$ and the
 * parameterization defined by quadpoints.
 * @param quadpoints This callback function defines a parameterization and it has
 * to return valid quadrature points, weight and normalvectors on the
 * parameterization.
 */
HEADER_PREFIX void setup_hmatrix_aprx_green_row_bem3d(pbem3d bem, pccluster rc,
    pccluster cc, pcblock tree, uint m, uint l, real delta,
    quadpoints3d quadpoints);

/**
 * @brief creating hmatrix approximation using green's method with column
 * @ref _cluster "cluster" .
 *
 * This function initializes the @ref _bem3d "bem3d" object for approximating
 * a matrix using green's formula. The approximation is based upon an expansion
 * around the column @ref _cluster "cluster".
 * For an admissible block @f$ b = (t,s) \in \mathcal L_{\mathcal I
 * \times \mathcal J}^+ @f$ one yields an approximation in shape of
 * @f[
 * G_{|t \times s} \approx A_{t,s} \, B_s^* \quad \text{with}
 * @f]
 * @f[
 * \left( A_{t,s}^1 \right)_{i\nu} := \omega_\nu \,  \int_\Gamma \,
 * \varphi_i(\vec x) \, \frac{\partial g}{\partial n_z} (\vec x,
 * \vec z_\nu) \, \mathrm d \vec x \quad ,
 * @f]
 * @f[
 * \left( A_{t,s}^2 \right)_{i\nu} := \int_\Gamma \, \varphi_i(\vec x) \,
 *  g(\vec x, \vec z_\nu) \, \mathrm d \vec x \quad ,
 * @f]
 * @f[
 * A_{t,s} = \begin{pmatrix} A^1_{t,s} & A^2_{t,s} \end{pmatrix} \quad ,
 * @f]
 *
 * @f[
 * \left( B_s^1 \right)_{j\nu} := \int_\Gamma  \,
 * \gamma (\vec z_\nu,
 * \vec y) \, \psi_j(\vec y) \, \mathrm d \vec y \quad ,
 * @f]
 * @f[
 * \left( B_s^2 \right)_{j\nu} := -\omega_\nu \, \int_\Gamma  \,
 * \frac{\partial \gamma}{\partial n_z}(\vec z_\nu,
 * \vec y) \, \psi_j(\vec y) \, \mathrm d \vec y \quad \text{and}
 * @f]
 * @f[
 * B_s = \begin{pmatrix} B_s^1 & B_s^2 \end{pmatrix} .
 * @f]
 * @f$ \gamma @f$ is the underlying kernel function defined by the the integral
 * operator. In case of single layer potential it applies @f$ \gamma = g @f$ but
 * in case of double layer potential it applies @f$ \gamma =
 * \frac{\partial g}{\partial n_y} @f$ .
 * The green quadrature points @f$ z_\nu @f$ and the weight @f$ \omega_\nu @f$
 * depend on the choice of <tt>quadpoints</tt> and <tt>delta</tt>, which define
 * the parameterization
 * around the row \ref _cluster "cluster".
 *
 * @param bem All needed callback functions and parameters for this approximation
 * scheme are set within the bem object.
 * @param rc Root of the row @ref _cluster "clustertree".
 * @param cc Root of the column @ref _cluster "clustertree".
 * @param tree Root of the @ref _block "blocktree".
 * @param m Number of gaussian quadrature points in each patch of the
 * parameterization.
 * @param l Number of subdivisions for gaussian quadrature in each patch of the
 * parameterization.
 * @param delta Defines the distance between the bounding box @f$ B_s @f$ and the
 * Parameterization defined by quadpoints.
 * @param quadpoints This callback function defines a parameterization and it has
 * to return valid quadrature points, weight and normalvectors on the
 * parameterization.
 */
HEADER_PREFIX void setup_hmatrix_aprx_green_col_bem3d(pbem3d bem, pccluster rc,
    pccluster cc, pcblock tree, uint m, uint l, real delta,
    quadpoints3d quadpoints);

/**
 * @brief creating hmatrix approximation using green's method with one of row or
 * column @ref _cluster "cluster" .
 *
 * This function initializes the @ref _bem3d "bem3d" object for approximating
 * a matrix using green's formula. The approximation is based upon an expansion
 * around the row or column @ref _cluster "cluster" depending on the diamter of
 * their bounding boxes..
 * For an admissible block @f$ b = (t,s) \in \mathcal L_{\mathcal I
 * \times \mathcal J}^+ @f$ one yields an approximation in shape of
 * @f[
 * G_{|t \times s} \approx \begin{cases} \begin{array}{ll}
 * A_{t,s} \, B_s^* &: \operatorname{diam}(t) < \operatorname{diam}(s)\\
 * A_t \, B_{t,s}^* &: \operatorname{diam}(t) \geq \operatorname{diam}(s)
 *  \end{array}
 *  \end{cases}
 * @f]
 * For definition of these matrices see @ref setup_hmatrix_aprx_green_row_bem3d and
 * @ref setup_hmatrix_aprx_green_col_bem3d .
 *
 * @param bem All needed callback functions and parameters for this approximation
 * scheme are set within the bem object.
 * @param rc Root of the row @ref _cluster "clustertree".
 * @param cc Root of the column @ref _cluster "clustertree".
 * @param tree Root of the @ref _block "blocktree".
 * @param m Number of gaussian quadrature points in each patch of the
 * parameterization.
 * @param l Number of subdivisions for gaussian quadrature in each patch of the
 * parameterization.
 * @param delta Defines the distance between the bounding box @f$ B_t @f$ or
 * @f$ B_s @f$ and the
 * Parameterization defined by quadpoints.
 * @param quadpoints This callback function defines a parameterization and it has
 * to return valid quadrature points, weight and normalvectors on the
 * parameterization.
 */
HEADER_PREFIX void setup_hmatrix_aprx_green_mixed_bem3d(pbem3d bem,
    pccluster rc, pccluster cc, pcblock tree, uint m, uint l, real delta,
    quadpoints3d quadpoints);

/* ------------------------------------------------------------
 Greenhybrid
 ------------------------------------------------------------ */

/**
 * @brief creating hmatrix approximation using green's method with row
 * @ref _cluster "cluster" connected with recompression technique using
 * ACA.
 *
 * This function initializes the @ref _bem3d "bem3d" object for approximating
 * a matrix using green's formula and ACA recompression.
 * The approximation is based upon an expansion
 * around the row @ref _cluster "cluster" and a recompression using adaptive
 * cross approximation.
 *
 * In a first step the algorithm creates the same matrix @f$ A_t @f$ as in
 * @ref setup_hmatrix_aprx_green_row_bem3d. From this matrix we create a new
 * approximation by using ACA with full pivoting (@ref decomp_fullaca_rkmatrix)
 * and we get a new rank-k-approximation of @f$ A_t @f$:
 * @f[
 * A_t \approx C_t \, D_t^* .
 * @f]
 * Afterwards we define an algebraic interpolation operator
 * @f[
 * I_t := C_t \, \left(R_t \, C_t \right)^{-1} \, R_t
 * @f]
 * with @f$ R_t @f$ selecting the pivot rows from the ACA-algorithm.
 * Appling @f$ I_t @f$ to our matrix block @f$ G_{|t \times s} @f$ we yield:
 * @f[
 *  G_{| t \times s} \approx I_t \, G_{| t \times s} =
 *  C_t \, \left(R_t \, C_t \right)^{-1} \, R_t \, G_{| t \times s} =
 *  V_t \, B_{t,s}^* \, .
 * @f]
 * This defines our final approximation of the block with the matrices
 * @f[
 * V_t := C_t \, \left(R_t \, C_t \right)^{-1}
 * @f]
 * and
 * @f[
 * B_{t,s} := \left( R_t \, G_{| t \times s} \right)^* \, .
 * @f]
 *
 * In order to save time and storage we compute the matrices @f$ V_t @f$ only once
 * for each cluster @f$ t \in \mathcal T_{\mathcal I} @f$.
 *
 * @param bem All needed callback functions and parameters for this approximation
 * scheme are set within the bem object.
 * @param rc Root of the row @ref _cluster "clustertree".
 * @param cc Root of the column @ref _cluster "clustertree".
 * @param tree Root of the @ref _block "blocktree".
 * @param m Number of gaussian quadrature points in each patch of the
 * parameterization.
 * @param l Number of subdivisions for gaussian quadrature in each patch of the
 * parameterization.
 * @param delta Defines the distance between the bounding box @f$ B_t @f$ and the
 * parameterization defined by quadpoints.
 * @param accur Assesses the minimum accuracy for the ACA approximation.
 * @param quadpoints This callback function defines a parameterization and it has
 * to return valid quadrature points, weight and normalvectors on the
 * parameterization.
 */
HEADER_PREFIX void setup_hmatrix_aprx_greenhybrid_row_bem3d(pbem3d bem,
    pccluster rc, pccluster cc, pcblock tree, uint m, uint l, real delta,
    real accur, quadpoints3d quadpoints);

/**
 * @brief creating hmatrix approximation using green's method with column
 * @ref _cluster "cluster" connected with recompression technique using
 * ACA.
 *
 * This function initializes the @ref _bem3d "bem3d" object for approximating
 * a matrix using green's formula and ACA recompression.
 * The approximation is based upon an expansion
 * around the column @ref _cluster "cluster" and a recompression using adaptive
 * cross approximation.
 *
 * In a first step the algorithm creates the same matrix @f$ B_s @f$ as in
 * @ref setup_hmatrix_aprx_green_col_bem3d. From this matrix we create a new
 * approximation by using ACA with full pivoting (@ref decomp_fullaca_rkmatrix)
 * and we get a new rank-k-approximation of @f$ B_s @f$:
 * @f[
 * B_s \approx C_s \, D_s^* .
 * @f]
 * Afterwards we define an algebraic interpolation operator
 * @f[
 * I_s := C_s \, \left(R_s \, C_s \right)^{-1} \, R_s
 * @f]
 * with @f$ R_s @f$ selecting the pivot rows from the ACA-algorithm.
 * Appling @f$ I_s @f$ to our matrix block @f$ G_{|t \times s} @f$ we yield:
 * @f[
 *  G_{| t \times s} \approx  G_{| t \times s} \, I_s^*  =
 *  G_{| t \times s} \, \left( C_s \, \left(R_s \, C_s \right)^{-1} \, R_s
 *  \right)^* = A_{t,s} \, W_s^* \, .
 * @f]
 * This defines our final approximation of the block with the matrices
 * @f[
 * A_{t,s} := G_{| t \times s} \, R_s^*
 * @f]
 * and
 * @f[
 * W_s := C_s \, \left(R_s \, C_s \right)^{-1} \, .
 * @f]
 *
 * In order to save time and storage we compute the matrices @f$ W_s @f$ only once
 * for each cluster @f$ s \in \mathcal T_{\mathcal J} @f$.
 *
 * @param bem All needed callback functions and parameters for this approximation
 * scheme are set within the bem object.
 * @param rc Root of the row @ref _cluster "clustertree".
 * @param cc Root of the column @ref _cluster "clustertree".
 * @param tree Root of the @ref _block "blocktree".
 * @param m Number of gaussian quadrature points in each patch of the
 * parameterization.
 * @param l Number of subdivisions for gaussian quadrature in each patch of the
 * parameterization.
 * @param delta Defines the distance between the bounding box @f$ B_s @f$ and the
 * parameterization defined by quadpoints.
 * @param accur Assesses the minimum accuracy for the ACA approximation.
 * @param quadpoints This callback function defines a parameterization and it has
 * to return valid quadrature points, weight and normalvectors on the
 * parameterization.
 */
HEADER_PREFIX void setup_hmatrix_aprx_greenhybrid_col_bem3d(pbem3d bem,
    pccluster rc, pccluster cc, pcblock tree, uint m, uint l, real delta,
    real accur, quadpoints3d quadpoints);

/**
 * @brief creating hmatrix approximation using green's method with row or column
 * @ref _cluster "cluster" connected with recompression technique using
 * ACA.
 *
 * This function initializes the @ref _bem3d "bem3d" object for approximating
 * a matrix using green's formula and ACA recompression.
 * The approximation is based upon an expansion
 * around the row or column @ref _cluster "cluster" and a recompression using adaptive
 * cross approximation.
 *
 * Depending on the rank produced by either @ref setup_hmatrix_aprx_greenhybrid_row_bem3d
 * or @ref setup_hmatrix_aprx_greenhybrid_col_bem3d we define this approximation by
 *
 * @f[
 * G_{|t \times s} \approx \begin{cases} \begin{array}{ll}
 *  V_t \, B_{t,s}* &: \operatorname{rank}(V_t) \leq \operatorname{rank}(W_s)\\
 *  A_{t,s} \, W_s* &: \operatorname{rank}(V_t) > \operatorname{rank}(W_s) \quad .
 * \end{array}
 * \end{cases}
 * @f]
 *
 * @param bem All needed callback functions and parameters for this approximation
 * scheme are set within the bem object.
 * @param rc Root of the row @ref _cluster "clustertree".
 * @param cc Root of the column @ref _cluster "clustertree".
 * @param tree Root of the @ref _block "blocktree".
 * @param m Number of gaussian quadrature points in each patch of the
 * parameterization.
 * @param l Number of subdivisions for gaussian quadrature in each patch of the
 * parameterization.
 * @param delta Defines the distance between the bounding box @f$ B_t @f$ or
 * @f$ B_s @f$ and the
 * parameterization defined by quadpoints.
 * @param accur Assesses the minimum accuracy for the ACA approximation.
 * @param quadpoints This callback function defines a parameterization and it has
 * to return valid quadrature points, weight and normalvectors on the
 * parameterization.
 */
HEADER_PREFIX void setup_hmatrix_aprx_greenhybrid_mixed_bem3d(pbem3d bem,
    pccluster rc, pccluster cc, pcblock tree, uint m, uint l, real delta,
    real accur, quadpoints3d quadpoints);

/* ------------------------------------------------------------
 ACA
 ------------------------------------------------------------ */

/**
 * @brief Approximate matrix block with ACA using full pivoting.
 *
 * This approximation scheme will utilize adaptive cross approximation with
 * full pivoting to retrieve a rank-k-approximation as
 * @f[
 * G_{|t \times s} \approx A_b \, B_b^*
 * @f]
 * with a given accuracy <tt>accur</tt>.
 * @param bem All needed callback functions and parameters for this approximation
 * scheme are set within the bem object.
 * @param rc Root of the row @ref _cluster "clustertree".
 * @param cc Root of the column @ref _cluster "clustertree".
 * @param tree Root of the @ref _block "blocktree".
 * @param accur Assesses the minimum accuracy for the ACA approximation.
 */
HEADER_PREFIX void setup_hmatrix_aprx_aca_bem3d(pbem3d bem, pccluster rc,
    pccluster cc, pcblock tree, real accur);

/**
 * @brief Approximate matrix block with ACA using partial pivoting.
 *
 * This approximation scheme will utilize adaptive cross approximation with
 * partial pivoting to retrieve a rank-k-approximation as
 * @f[
 * G_{|t \times s} \approx A_b \, B_b^*
 * @f]
 * with a given accuracy <tt>accur</tt>.
 * @param bem All needed callback functions and parameters for this approximation
 * scheme are set within the bem object.
 * @param rc Root of the row @ref _cluster "clustertree".
 * @param cc Root of the column @ref _cluster "clustertree".
 * @param tree Root of the @ref _block "blocktree".
 * @param accur Assesses the minimum accuracy for the ACA approximation.
 */
HEADER_PREFIX void setup_hmatrix_aprx_paca_bem3d(pbem3d bem, pccluster rc,
    pccluster cc, pcblock tree, real accur);

/* ------------------------------------------------------------
 HCA
 ------------------------------------------------------------ */

/**
 * @brief Approximate matrix block with hybrid cross approximation using
 * Interpolation and ACA with full pivoting.
 *
 * At first for a block @f$ b = (t,s) \in \mathcal L_{\mathcal I \times
 * \mathcal J}^+ @f$ the matrix @f$ S @f$ with @f$ S_{\mu\nu} =
 * g(\vec \xi_\mu, \vec \xi\nu) @f$ is set up with @f$ \vec \xi_\mu @f$
 * interpolation points from @f$ B_t @f$ and @f$ \vec \xi_\nu @f$
 * interpolation points from @f$ B_s @f$ . Then applying ACA with full pivoting
 * to @f$ S @f$ yields the representation:
 * @f[
 * S_{\mu\nu} \approx \sum_{\alpha = 1}^k \sum_{\beta = 1}^k \, S_{\mu \alpha} \, \hat S_{\alpha \beta} \,
 * S_{\beta \nu}
 * @f]
 * with @f$ \hat S := \left( S_{|I_t \times I_s} \right)^{-1} @f$ and @f$ I_t \, , I_s @f$
 * defining the pivot rows and columns from the ACA algoritm.
 * This means we can rewrite the fundamental solution @f$ g @f$ as follows:
 * @f[
 * g(\vec x, \vec y) \approx \sum_{\mu = 1}^{m^3 }\sum_{\nu = 1}^{m^3}
 * \mathcal L_\mu(\vec x) \, g(\vec \xi_\mu, \vec \xi\nu) \, \mathcal L_\nu(\vec y)
 * \approx \sum_{\mu = 1}^{m^3 }\sum_{\nu = 1}^{m^3} \sum_{\alpha = 1}^k \sum_{\beta = 1}^k
 * \mathcal L_\mu(\vec x) \, S_{\mu \alpha} \, \hat S_{\alpha \beta} \,
 * S_{\beta \nu} \, \mathcal L_\nu(\vec y)
 * @f]
 * Now reversing the interpolation we can express this formula as:
 * @f[
 * g(\vec x, \vec y) \approx \sum_{\alpha = 1}^k \sum_{\beta = 1}^k \, \hat S_{\alpha \beta} \,
 * \sum_{\mu = 1}^{m^3 } \, \left( \mathcal L_\mu(\vec x) \, S_{\mu \alpha} \right) \,
 * \sum_{\nu = 1}^{m^3} \left( S_{\beta \nu} \, \mathcal L_\nu(\vec y) \right)
 * \approx \sum_{\alpha = 1}^k \sum_{\beta = 1}^k \,
 * g(\vec x, \vec \xi_\alpha) \, \hat S_{\alpha \beta} \,  g(\vec \xi_\beta, \vec y)
 * @f]
 * Depending on the condition @f$ \#t < \#s \text{ or } \#t \geq \#s @f$
 * we can now construct the rank-k-approximation as
 * @f[
 * G_{|t \times s} \approx \left( A_b \, \hat S \right) \, B_b^* \quad \text{or}
 * @f]
 * @f[
 * G_{|t \times s} \approx A_b \, \left( B_b \,\hat S \right)^*
 * @f]
 * with matrices
 * @f[
 * \left( A_b \right)_{i\alpha} := \int_\Gamma \, \varphi(\vec x) \, g(\vec x,
 * \vec \xi_\alpha) \, \mathrm d \vec x \quad ,
 * @f]
 * @f[
 * \left( B_b \right)_{j\beta} := \int_\Gamma \, g(\vec \xi_\beta,
 * \vec y) \, \psi(\vec y) \, \mathrm d \vec y \quad .
 * @f]
 *
 * @param bem All needed callback functions and parameters for this approximation
 * scheme are set within the bem object.
 * @param rc Root of the row @ref _cluster "clustertree".
 * @param cc Root of the column @ref _cluster "clustertree".
 * @param tree Root of the @ref _block "blocktree".
 * @param m Number of Chebyshev interpolation points in each spatial dimension.
 * @param accur Assesses the minimum accuracy for the ACA approximation.
 */
HEADER_PREFIX void setup_hmatrix_aprx_hca_bem3d(pbem3d bem, pccluster rc,
    pccluster cc, pcblock tree, uint m, real accur);

/* ------------------------------------------------------------
 Initializerfunctions for h2-matrix approximations
 ------------------------------------------------------------ */

/**
 * @brief Enables hierarchical recompression for @ref _hmatrix "hmatrices".
 *
 * This function enables hierarchical recompression. It is only necessary to
 * initialize the @ref _bem3d "bem3d" object with some @ref _hmatrix "hmatrix"
 * compression technique such as @ref setup_hmatrix_aprx_inter_row_bem3d . Afterwards
 * one just has to call @ref assemblehiercomp_bem3d_h2matrix to create a initially
 * compressed @ref _h2matrix "h2matrix" approximation.
 *
 * @param bem All needed callback functions and parameters for h2-recompression
 * are set within the bem object.
 * @param hiercomp A flag to indicates whether hierarchical recompression should
 * be used or not.
 * @param accur_hiercomp The accuracy the hierarchical recompression technique
 * will use to create a compressed @ref _h2matrix "h2matrix.
 */
HEADER_PREFIX void setup_h2matrix_recomp_bem3d(pbem3d bem, bool hiercomp,
    real accur_hiercomp);

/**
 * @brief Initialize the @ref _bem3d "bem3d" object for approximating
 * a @ref _h2matrix "h2matrix" with tensorinterpolation.
 *
 * In case of leaf cluster @f$ t, s @f$ this approximation scheme creates
 * a @ref _h2matrix "h2matrix" approximation of the following form:
 * @f[
 * G_{| t \times s} \approx V_t \, S_b \, W_s^*
 * @f]
 * with the matrices defined as
 * @f[
 * \left( V_t \right)_{i\mu} := \int_\Gamma \, \varphi_i(\vec x) \,
 * \mathcal L_\mu(\vec x) \, \mathrm d \vec x
 * @f]
 * @f[
 * \left( W_s \right)_{j\nu} := \int_\Gamma  \,
 * \Lambda_\nu(\vec y) \, \psi_j(\vec y) \, \mathrm d \vec y
 * @f]
 * @f[
 * \left( S_b \right)_{\mu\nu} := g(\vec \xi_\mu, \vec \xi_\nu) \, .
 * @f]
 * Here @f$ \Lambda_\nu @f$ depends on the choice of the integral operator: in case
 * of the single layer potential it applies @f$ \Lambda_\nu = \mathcal L_\nu @f$
 * and in case of the double layer potential it applies
 * @f$ \Lambda_\nu = \frac{\partial \mathcal L_\nu}{\partial n} @f$ .
 *
 * In case of non leaf clusters transfer matrices @f$ E_{t_1} \ldots E_{t_\tau} @f$
 * and @f$ F_{s_1} \ldots F_{s_\sigma} @f$ are also constructed and the approximation
 * of the block looks like:
 * @f[
 * G_{| t \times s} \approx \begin{pmatrix} V_{t_1} & & \\ & \ddots & \\
 * & & V_{t_\tau} \end{pmatrix} \, \begin{pmatrix} E_{t_1} \\ \vdots \\ E_{t_\tau}
 * \end{pmatrix} S_b \, \begin{pmatrix} F_{s_1}^* & \cdots & F_{s_\sigma}^*
 * \end{pmatrix} \, \begin{pmatrix} W_{s_1}^* & & \\ & \ddots & \\
 * & & W_{s_\sigma}^* \end{pmatrix}
 * @f]
 * The transfer matrices look like
 * @f[
 * \left( E_{t_i} \right)_{\mu\mu'} := \mathcal L_{\mu}(\xi_{\mu'})
 * @f]
 * respectively
 * @f[
 * \left( F_{s_j} \right)_{\nu\nu'} := \mathcal L_{\nu}(\xi_{\nu'})
 * @f]
 *
 * @param bem All needed callback functions and parameters for this approximation
 * scheme are set within the bem object.
 * @param rb Root of the row @ref _clusterbasis "clusterbasis".
 * @param cb Root of the column @ref _clusterbasis "clusterbasis".
 * @param tree Root of the @ref _block "blocktree".
 * @param m Number of Chebyshev interpolation points in each spatial dimension.
 */
HEADER_PREFIX void setup_h2matrix_aprx_inter_bem3d(pbem3d bem,
    pcclusterbasis rb, pcclusterbasis cb, pcblock tree, uint m);

/**
 * @brief  Initialize the @ref _bem3d "bem3d" object for approximating
 * a @ref _h2matrix "h2matrix" with green's method and ACA based
 * recompression.
 *
 * In case of leaf clusters, we use the techniques from
 * @ref setup_hmatrix_aprx_greenhybrid_row_bem3d and
 * @ref setup_hmatrix_aprx_greenhybrid_col_bem3d and construct two algebraic
 * interpolation operators @f$ I_t @f$ and @f$ I_s @f$ and approximate an admissible
 * block @f$ b = (t,s) \in \mathcal L_{\mathcal I \times \mathcal J}^+ @f$ by
 * @f[
 * G_{| t \times s} \approx I_t \, G_{| t \times s} \, I_s^* \, .
 * @f]
 * This yields the representation:
 * @f[
 * G_{| t \times s} \approx I_t \, G_{| t \times s} \, I_s^*
 * = V_t \, R_t \, G_{| t \times s} \, R_s^* \, W_s^* = V_t \, S_b \, W_s^*
 * @f]
 * with @f$ S_b := R_t \, G_{| t \times s} \, R_s^* @f$ just a
 * @f$ k \times k @f$-matrix taking just the pivot rows and columns of the original
 * matrix @f$ G_{| t \times s} @f$.
 *
 * In case of non leaf clusters transfer matrices @f$ E_{t_1} \ldots E_{t_\tau} @f$
 * and @f$ F_{s_1} \ldots F_{s_\sigma} @f$ are also introduced and the approximation
 * of the block looks like:
 * @f[
 * G_{| t \times s} \approx \begin{pmatrix} V_{t_1} & & \\ & \ddots & \\
 * & & V_{t_\tau} \end{pmatrix} \, \begin{pmatrix} E_{t_1} \\ \vdots \\ E_{t_\tau}
 * \end{pmatrix} S_b \, \begin{pmatrix} F_{s_1}^* & \cdots & F_{s_\sigma}^*
 * \end{pmatrix} \, \begin{pmatrix} W_{s_1}^* & & \\ & \ddots & \\
 * & & W_{s_\sigma}^* \end{pmatrix}
 * @f]
 * The transfer matrices are build up in the following way: At first for
 * a non leaf cluster @f$ t \in \mathcal T_{\mathcal I} @f$ we construct a matrix
 * @f$ A_t @f$ as in @ref setup_hmatrix_aprx_green_row_bem3d but not with the index
 * set @f$ \hat t @f$ but with the index set
 * @f[
 * \bigcup_{t' \in \operatorname{sons}(t)} \, R_{t'} \, .
 * @f]
 * Then again we use adaptive cross approximation technique to receive
 * @f[
 * A_t \approx C_t \, D_t^* \, .
 * @f]
 * And again we construct an interpolation operator as before:
 * @f[
 * I_t := C_t \left( R_t \,  C_t \right)^{-1} \, R_t
 * @f]
 * And finally we have to solve
 * @f[
 * \begin{pmatrix} E_{t_1} \\ \vdots \\ E_{t_\tau} \end{pmatrix} =
 * C_t \left( R_t \,  C_t \right)^{-1}
 * @f]
 * for our desired @f$ E_{t'} @f$.<br>
 * Respectively the transfer matrices for the column clusterbasis are constructed.
 *
 * @see setup_hmatrix_aprx_greenhybrid_row_bem3d
 * @see setup_hmatrix_aprx_greenhybrid_col_bem3d
 *
 * @param bem All needed callback functions and parameters for this approximation
 * scheme are set within the bem object.
 * @param rb Root of the row @ref _clusterbasis "clusterbasis".
 * @param cb Root of the column @ref _clusterbasis "clusterbasis".
 * @param tree Root of the @ref _block "blocktree".
 * @param m Number of gaussian quadrature points in each patch of the
 * parameterization.
 * @param l Number of subdivisions for gaussian quadrature in each patch of the
 * parameterization.
 * @param delta Defines the distance between the bounding box @f$ B_t @f$ or
 * @f$ B_s @f$ and the
 * Parameterization defined by quadpoints.
 * @param accur Assesses the minimum accuracy for the ACA approximation.
 * @param quadpoints This callback function defines a parameterization and it has
 * to return valid quadrature points, weight and normalvectors on the
 * parameterization.
 */
HEADER_PREFIX void setup_h2matrix_aprx_greenhybrid_bem3d(pbem3d bem,
    pcclusterbasis rb, pcclusterbasis cb, pcblock tree, uint m, uint l,
    real delta, real accur, quadpoints3d quadpoints);

/**
 * @brief  Initialize the @ref _bem3d "bem3d" object for approximating
 * a @ref _h2matrix "h2matrix" with green's method and ACA based
 * recompression. The resulting @ref _clusterbasis "clusterbasis" will be orthogonal.
 *
 * In case of leaf clusters, we use the techniques similar to
 * @ref setup_hmatrix_aprx_greenhybrid_row_bem3d and
 * @ref setup_hmatrix_aprx_greenhybrid_col_bem3d and construct two algebraic
 * interpolation operators @f$ I_t @f$ and @f$ I_s @f$ and approximate an admissible
 * block @f$ b = (t,s) \in \mathcal L_{\mathcal I \times \mathcal J}^+ @f$ by
 * @f[
 * G_{| t \times s} \approx I_t \, G_{| t \times s} \, I_s^* \, .
 * @f]
 * This yields the representation:
 * @f[
 * G_{| t \times s} \approx I_t \, G_{| t \times s} \, I_s^*
 * = V_t \, R_t \, G_{| t \times s} \, R_s^* \, W_s^* = V_t \, S_b \, W_s^*
 * @f]
 * with @f$ S_b := R_t \, G_{| t \times s} \, R_s^* @f$ just a
 * @f$ k \times k @f$-matrix taking just the pivot rows and columns of the original
 * matrix @f$ G_{| t \times s} @f$.
 *
 * In case of non leaf clusters transfer matrices @f$ E_{t_1} \ldots E_{t_\tau} @f$
 * and @f$ F_{s_1} \ldots F_{s_\sigma} @f$ are also introduced and the approximation
 * of the block looks like:
 * @f[
 * G_{| t \times s} \approx \begin{pmatrix} V_{t_1} & & \\ & \ddots & \\
 * & & V_{t_\tau} \end{pmatrix} \, \begin{pmatrix} E_{t_1} \\ \vdots \\ E_{t_\tau}
 * \end{pmatrix} S_b \, \begin{pmatrix} F_{s_1}^* & \cdots & F_{s_\sigma}^*
 * \end{pmatrix} \, \begin{pmatrix} W_{s_1}^* & & \\ & \ddots & \\
 * & & W_{s_\sigma}^* \end{pmatrix}
 * @f]
 * The transfer matrices are build up in the following way: At first for
 * a non leaf cluster @f$ t \in \mathcal T_{\mathcal I} @f$ we construct a matrix
 * @f$ A_t @f$ as in @ref setup_hmatrix_aprx_green_row_bem3d but not with the index
 * set @f$ \hat t @f$ but with the index set
 * @f[
 * \bigcup_{t' \in \operatorname{sons}(t)} \, R_{t'} \, .
 * @f]
 * Then again we use adaptive cross approximation technique to receive
 * @f[
 * A_t \approx C_t \, D_t^* \, .
 * @f]
 * And again we construct an interpolation operator as before:
 * @f[
 * I_t := C_t \left( R_t \,  C_t \right)^{-1} \, R_t
 * @f]
 * And finally we have to solve
 * @f[
 * \begin{pmatrix} E_{t_1} \\ \vdots \\ E_{t_\tau} \end{pmatrix} =
 * C_t \left( R_t \,  C_t \right)^{-1}
 * @f]
 * for our desired @f$ E_{t'} @f$.<br>
 * Respectively the transfer matrices for the column clusterbasis are constructed.
 *
 * @see setup_hmatrix_aprx_greenhybrid_row_bem3d
 * @see setup_hmatrix_aprx_greenhybrid_col_bem3d
 *
 * @param bem All needed callback functions and parameters for this approximation
 * scheme are set within the bem object.
 * @param rb Root of the row @ref _clusterbasis "clusterbasis".
 * @param cb Root of the column @ref _clusterbasis "clusterbasis".
 * @param tree Root of the @ref _block "blocktree".
 * @param m Number of gaussian quadrature points in each patch of the
 * parameterization.
 * @param l Number of subdivisions for gaussian quadrature in each patch of the
 * parameterization.
 * @param delta Defines the distance between the bounding box @f$ B_t @f$ or
 * @f$ B_s @f$ and the
 * Parameterization defined by quadpoints.
 * @param accur Assesses the minimum accuracy for the ACA approximation.
 * @param quadpoints This callback function defines a parameterization and it has
 * to return valid quadrature points, weight and normalvectors on the
 * parameterization.
 */
HEADER_PREFIX void setup_h2matrix_aprx_greenhybrid_ortho_bem3d(pbem3d bem,
    pcclusterbasis rb, pcclusterbasis cb, pcblock tree, uint m, uint l,
    real delta, real accur, quadpoints3d quadpoints);

/* ------------------------------------------------------------
 Fill hmatrix
 ------------------------------------------------------------ */

/**
 * @brief Fills an @ref _hmatrix "hmatrix" with a predefined approximation
 * technique.
 *
 * This will traverse the block tree until it reaches its leafs.
 * For a leaf block either the full matrix @f$
 * G_{|t \times s} @f$ is computed or a rank-k-approximation specified within the
 * @ref _bem3d "bem3d" object is created as @f$ G_{|t \times s} \approx A_b
 * \, B_b^* @f$
 *
 * @attention Before using this function to fill an @ref _hmatrix "hmatrix" one has to
 * initialize the @ref _bem3d "bem3d" object with one of the approximation
 * techniques such as @ref setup_hmatrix_aprx_inter_row_bem3d. Otherwise the
 * behavior of the function is undefined.
 *
 * @param bem @ref _bem3d "bem3d" object containing all necessary information
 * for computing the entries of @ref _hmatrix "hmatrix" <tt>G</tt> .
 * @param b Root of the @ref _block "blocktree".
 * @param G @ref _hmatrix "hmatrix" to be filled. <tt>b</tt> has to be
 * appropriate to <tt>G</tt>.
 */
HEADER_PREFIX void assemble_bem3d_hmatrix(pbem3d bem, pblock b, phmatrix G);

/**
 * @brief Fills an @ref _hmatrix "hmatrix" with a predefined approximation
 * technique using coarsening strategy.
 *
 * This will traverse the block tree until it reaches its leafs.
 * For a leaf block either the full matrix @f$
 * G_{|t \times s} @f$ is computed or a rank-k-approximation specified within the
 * @ref _bem3d "bem3d" object is created as @f$ G_{|t \times s} \approx A_b
 * \, B_b^* @f$
 * While traversing the block tree backwards coarsening algorithm determines
 * whether current non leaf block could be stored more efficiently as a single
 * compressed leaf block in rank-k-format or not.
 *
 * @attention Before using this function to fill an @ref _hmatrix "hmatrix" one has to
 * initialize the @ref _bem3d "bem3d" object with one of the approximation
 * techniques such as @ref setup_hmatrix_aprx_inter_row_bem3d. Otherwise the
 * behavior of the function is undefined.
 * @attention Make sure to call @ref setup_hmatrix_recomp_bem3d  with enabled coarsening
 * and appropriate accuracy before calling this function. Default value of
 * coarsening accuracy is set to 0.0, this means no coarsening will be done if
 * a realistic accuracy is not set within the @ref _bem3d "bem3d" object.
 *
 * @param bem @ref _bem3d "bem3d" object containing all necessary information
 * for computing the entries of @ref _hmatrix "hmatrix" <tt>G</tt> .
 * @param b Root of the @ref _block "blocktree".
 * @param G @ref _hmatrix "hmatrix" to be filled. <tt>b</tt> has to be
 * appropriate to <tt>G</tt>.
 */
HEADER_PREFIX void assemblecoarsen_bem3d_hmatrix(pbem3d bem, pblock b,
    phmatrix G);

/* ------------------------------------------------------------
 Fill h2-matrix
 ------------------------------------------------------------ */

/**
 * @brief This function computes the matrix entries for the nested @ref _clusterbasis
 * "clusterbasis" @f$ V_t \, , t \in \mathcal T_{\mathcal I} @f$.
 *
 * This algorithm completely traverses the clustertree belonging to the given
 * @ref _clusterbasis "clusterbasis" until it reaches its leafs. In each leaf cluster
 * the matrix @f$ V_t @f$ will be constructed and stored within the @ref
 * _clusterbasis "clusterbasis". In non leaf clusters the transfer matrices
 * @f$ E_{t'} \, , t' \in \operatorname{sons}(t) @f$ will be computed and it holds
 * @f[
 * V_t = \begin{pmatrix} V_{t_1} & & \\ & \ddots & \\ & & V_{t_\tau} \end{pmatrix}
 * \begin{pmatrix} E_{t_1} \\ \vdots \\ E_{t_\tau} \end{pmatrix}
 * @f]
 * A matrices @f$ E_{t_i} @f$ will be stored within the @ref _clusterbasis
 * "clusterbasis" belonging to the @f$ i @f$-th son.
 *
 * @attention A valid @ref _h2matrix "h2matrix" approximation scheme,
 * such as @ref setup_h2matrix_aprx_inter_bem3d must be set before calling
 * this function. If no approximation technique is set within the @ref _bem3d
 * "bem3d" object the behavior of this function is undefined.
 *
 * @param bem @ref _bem3d "bem3d" object containing all necessary information
 * for computing the entries of @ref _clusterbasis "clusterbasis" <tt>rb</tt> .
 * @param rb Row @ref _clusterbasis "clusterbasis" to be filled.
 */
HEADER_PREFIX void assemble_bem3d_h2matrix_row_clusterbasis(pcbem3d bem,
    pclusterbasis rb);

/**
 * @brief This function computes the matrix entries for the nested @ref _clusterbasis
 * "clusterbasis" @f$ W_s \, , s \in \mathcal T_{\mathcal J} @f$.
 *
 * This algorithm completely traverses the clustertree belonging to the given
 * @ref _clusterbasis "clusterbasis" until it reaches its leafs. In each leaf cluster
 * the matrix @f$ W_s @f$ will be constructed and stored within the @ref
 * _clusterbasis "clusterbasis". In non leaf clusters the transfer matrices
 * @f$ F_{s'} \, , s' \in \operatorname{sons}(s) @f$ will be computed and it holds
 * @f[
 * W_s = \begin{pmatrix} W_{s_1} & & \\ & \ddots & \\ & & W_{s_\sigma} \end{pmatrix}
 * \begin{pmatrix} F_{s_1} \\ \vdots \\ F_{s_\sigma} \end{pmatrix}
 * @f]
 * A matrices @f$ F_{s_i} @f$ will be stored within the @ref _clusterbasis
 * "clusterbasis" belonging to the @f$ i @f$-th son.
 *
 * @attention A valid @ref _h2matrix "h2matrix" approximation scheme,
 * such as @ref setup_h2matrix_aprx_inter_bem3d must be set before calling
 * this function. If no approximation technique is set within the @ref _bem3d
 * "bem3d" object the behavior of this function is undefined.
 *
 * @param bem @ref _bem3d "bem3d" object containing all necessary information
 * for computing the entries of @ref _clusterbasis "clusterbasis" <tt>rb</tt> .
 * @param cb Row @ref _clusterbasis "clusterbasis" to be filled.
 */
HEADER_PREFIX void assemble_bem3d_h2matrix_col_clusterbasis(pcbem3d bem,
    pclusterbasis cb);

/**
 * @brief Fills an @ref _h2matrix "h2matrix" with a predefined approximation
 * technique.
 *
 * This will traverse the block tree until it reaches its leafs.
 * For a leaf block either the full matrix @f$
 * G_{|t \times s} @f$ is computed or a @ref _uniform "uniform"
 * rank-k-approximation specified
 * within the @ref _bem3d "bem3d" object is created as
 * @f$ G_{|t \times s} \approx V_t \, S_b \, W_s^* @f$
 *
 * @attention Before using this function to fill an @ref _h2matrix
 * "h2matrix" one has to initialize the @ref _bem3d "bem3d" object with
 * one of the approximation techniques such as @ref
 * setup_h2matrix_aprx_inter_bem3d. Otherwise the
 * behavior of the function is undefined.
 * @attention The @ref _clusterbasis "clusterbasis" @f$ V_t @f$ and @f$ W_s @f$
 * have to be computed before
 * calling this function because some approximation schemes depend on information
 * residing within the @ref _clusterbasis "clusterbasis".
 *
 * @param bem @ref _bem3d "bem3d" object containing all necessary information
 * for computing the entries of @ref _h2matrix "h2matrix" <tt>G</tt> .
 * @param b Root of the @ref _block "blocktree".
 * @param G @ref _h2matrix "h2matrix" to be filled. <tt>b</tt> has to be
 * appropriate to <tt>G</tt>.
 */
HEADER_PREFIX void assemble_bem3d_h2matrix(pbem3d bem, pblock b, ph2matrix G);

/**
 * @brief Fills an @ref _h2matrix "h2matrix" with a predefined approximation
 * technique using hierarchical recompression.
 *
 * This functions does not need to initialized by an appropriate approximation
 * technique for @ref _h2matrix "h2matrices". Instead one needs to specify a
 * valid approximation scheme for @ref _hmatrix "hmatrices" such as
 * @ref setup_hmatrix_aprx_inter_row_bem3d.
 * When this method is finished not only the @ref _h2matrix "h2matrix" <tt>G</tt>
 * is filled but also the corresponding row and column @ref _clusterbasis
 * "clusterbasis".
 *
 * @attention Before using this function to fill an @ref _h2matrix
 * "h2matrix" one has to initialize the @ref _bem3d "bem3d" object with
 * one of the @ref _hmatrix "hmatrix" approximation techniques such as @ref
 * setup_hmatrix_aprx_inter_row_bem3d. Otherwise the
 * behavior of the function is undefined.
 * @attention Storage for the @ref _clusterbasis "clusterbasis" @f$ V_t @f$
 * and @f$ W_s @f$  has to be allocated before calling this function.
 * @attention Before calling this function @ref setup_h2matrix_recomp_bem3d has
 * to be called enabling the hierarchical recompression with a valid
 * accuracy. If this is not done, the default accuracy is set to 0.0. This will
 * yield no reasonable approximation according to the resulting size of the
 * @ref _h2matrix "h2matrix.
 *
 * @param bem @ref _bem3d "bem3d" object containing all necessary information
 * for computing the entries of @ref _h2matrix "h2matrix" <tt>G</tt> .
 * @param b Root of the @ref _block "blocktree".
 * @param G @ref _h2matrix "h2matrix" to be filled. <tt>b</tt> has to be
 * appropriate to <tt>G</tt>.
 */
HEADER_PREFIX void assemblehiercomp_bem3d_h2matrix(pbem3d bem, pblock b,
    ph2matrix G);

/* ------------------------------------------------------------
 lagrange-polynomials
 ------------------------------------------------------------ */

/**
 * @brief This function will integrate Lagrange polynomials on the boundary domain
 * using piecewise constant basis function and store the results in a matrix
 * <tt>V</tt>.
 *
 * The matrix entries of <tt>V</tt> will be computed as
 * @f[
 * \left( V \right)_{i\mu} := \int_\Gamma \, \varphi_i(\vec x) \, \mathcal
 * L_\mu (\vec x) \, \mathrm d \vec x
 * @f]
 * with Lagrange polynomial @f$ \mathcal L_\mu (\vec x) @f$ defined as:
 * @f[
 * \mathcal L_\mu (\vec x) := \left( \prod_{\nu_1 = 1, \nu_1 \neq \mu_1}^m
 * \frac{\xi_{\nu_1} - x_1}{\xi_{\nu_1} - \xi_{\mu_1}} \right) \cdot
 * \left( \prod_{\nu_2 = 1, \nu_2 \neq \mu_2}^m
 * \frac{\xi_{\nu_2} - x_2}{\xi_{\nu_2} - \xi_{\mu_2}} \right) \cdot
 * \left( \prod_{\nu_3 = 1, \nu_3 \neq \mu_3}^m
 * \frac{\xi_{\nu_3} - x_3}{\xi_{\nu_3} - \xi_{\mu_3}} \right)
 * @f]
 *
 * @param idx This array describes the permutation of the degrees of freedom.
 * Its length is determined by <tt>V->rows</tt> . In case <tt>idx == NULL</tt>
 * it is assumed the degrees of freedom are @f$ 0, 1, \ldots , \texttt{V->rows}
 *  -1 @f$ instead.
 * @param px A Vector of type @ref _avector "avector" that contains interpolation
 * points in x-direction.
 * @param py A Vector of type @ref _avector "avector" that contains interpolation
 * points in y-direction.
 * @param pz A Vector of type @ref _avector "avector" that contains interpolation
 * points in z-direction.
 * @param bem BEM-object containing additional information for computation
 * of the matrix entries.
 * @param V Matrix to store the computed results as stated above.
 */
HEADER_PREFIX void assemble_bem3d_lagrange_const_amatrix(const uint *idx,
    pcavector px, pcavector py, pcavector pz, pcbem3d bem, pamatrix V);

/**
 * @brief This function will integrate Lagrange polynomials on the boundary domain
 * using linear basis function and store the results in a matrix
 * <tt>V</tt>.
 *
 * The matrix entries of <tt>V</tt> will be computed as
 * @f[
 * \left( V \right)_{i\mu} := \int_\Gamma \, \varphi_i(\vec x) \, \mathcal
 * L_\mu (\vec x) \, \mathrm d \vec x
 * @f]
 * with Lagrange polynomial @f$ \mathcal L_\mu (\vec x) @f$ defined as:
 * @f[
 * \mathcal L_\mu (\vec x) := \left( \prod_{\nu_1 = 1, \nu_1 \neq \mu_1}^m
 * \frac{\xi_{\nu_1} - x_1}{\xi_{\nu_1} - \xi_{\mu_1}} \right) \cdot
 * \left( \prod_{\nu_2 = 1, \nu_2 \neq \mu_2}^m
 * \frac{\xi_{\nu_2} - x_2}{\xi_{\nu_2} - \xi_{\mu_2}} \right) \cdot
 * \left( \prod_{\nu_3 = 1, \nu_3 \neq \mu_3}^m
 * \frac{\xi_{\nu_3} - x_3}{\xi_{\nu_3} - \xi_{\mu_3}} \right)
 * @f]
 *
 * @param idx This array describes the permutation of the degrees of freedom.
 * Its length is determined by <tt>V->rows</tt> . In case <tt>idx == NULL</tt>
 * it is assumed the degrees of freedom are @f$ 0, 1, \ldots , \texttt{V->rows}
 *  -1 @f$ instead.
 * @param px A Vector of type @ref _avector "avector" that contains interpolation
 * points in x-direction.
 * @param py A Vector of type @ref _avector "avector" that contains interpolation
 * points in y-direction.
 * @param pz A Vector of type @ref _avector "avector" that contains interpolation
 * points in z-direction.
 * @param bem BEM-object containing additional information for computation
 * of the matrix entries.
 * @param V Matrix to store the computed results as stated above.
 */
HEADER_PREFIX void assemble_bem3d_lagrange_linear_amatrix(const uint *idx,
    pcavector px, pcavector py, pcavector pz, pcbem3d bem, pamatrix V);

/**
 * @brief This function will integrate the normal derivatives of the Lagrange
 * polynomials on the boundary domain using piecewise constant basis function
 * and store the results in a matrix <tt>V</tt>.
 *
 * The matrix entries of <tt>V</tt> will be computed as
 * @f[
 * \left( V \right)_{i\mu} := \int_\Gamma \, \varphi_i(\vec x) \,
 * \frac{\partial \mathcal L_\mu}{\partial n} (\vec x) \, \mathrm d \vec x
 * @f]
 * with Lagrange polynomial @f$ \mathcal L_\mu (\vec x) @f$ defined as:
 * @f[
 * \mathcal L_\mu (\vec x) := \left( \prod_{\nu_1 = 1, \nu_1 \neq \mu_1}^m
 * \frac{\xi_{\nu_1} - x_1}{\xi_{\nu_1} - \xi_{\mu_1}} \right) \cdot
 * \left( \prod_{\nu_2 = 1, \nu_2 \neq \mu_2}^m
 * \frac{\xi_{\nu_2} - x_2}{\xi_{\nu_2} - \xi_{\mu_2}} \right) \cdot
 * \left( \prod_{\nu_3 = 1, \nu_3 \neq \mu_3}^m
 * \frac{\xi_{\nu_3} - x_3}{\xi_{\nu_3} - \xi_{\mu_3}} \right)
 * @f]
 *
 * @param idx This array describes the permutation of the degrees of freedom.
 * Its length is determined by <tt>V->rows</tt> . In case <tt>idx == NULL</tt>
 * it is assumed the degrees of freedom are @f$ 0, 1, \ldots , \texttt{V->rows}
 *  -1 @f$ instead.
 * @param px A Vector of type @ref _avector "avector" that contains interpolation
 * points in x-direction.
 * @param py A Vector of type @ref _avector "avector" that contains interpolation
 * points in y-direction.
 * @param pz A Vector of type @ref _avector "avector" that contains interpolation
 * points in z-direction.
 * @param bem BEM-object containing additional information for computation
 * of the matrix entries.
 * @param V Matrix to store the computed results as stated above.
 */
HEADER_PREFIX void assemble_bem3d_dn_lagrange_const_amatrix(const uint *idx,
    pcavector px, pcavector py, pcavector pz, pcbem3d bem, pamatrix V);

/**
 * @brief This function will integrate the normal derivatives of the Lagrange
 * polynomials on the boundary domain using linear basis function
 * and store the results in a matrix <tt>V</tt>.
 *
 * The matrix entries of <tt>V</tt> will be computed as
 * @f[
 * \left( V \right)_{i\mu} := \int_\Gamma \, \varphi_i(\vec x) \,
 * \frac{\partial \mathcal L_\mu}{\partial n} (\vec x) \, \mathrm d \vec x
 * @f]
 * with Lagrange polynomial @f$ \mathcal L_\mu (\vec x) @f$ defined as:
 * @f[
 * \mathcal L_\mu (\vec x) := \left( \prod_{\nu_1 = 1, \nu_1 \neq \mu_1}^m
 * \frac{\xi_{\nu_1} - x_1}{\xi_{\nu_1} - \xi_{\mu_1}} \right) \cdot
 * \left( \prod_{\nu_2 = 1, \nu_2 \neq \mu_2}^m
 * \frac{\xi_{\nu_2} - x_2}{\xi_{\nu_2} - \xi_{\mu_2}} \right) \cdot
 * \left( \prod_{\nu_3 = 1, \nu_3 \neq \mu_3}^m
 * \frac{\xi_{\nu_3} - x_3}{\xi_{\nu_3} - \xi_{\mu_3}} \right)
 * @f]
 *
 * @param idx This array describes the permutation of the degrees of freedom.
 * Its length is determined by <tt>V->rows</tt> . In case <tt>idx == NULL</tt>
 * it is assumed the degrees of freedom are @f$ 0, 1, \ldots , \texttt{V->rows}
 *  -1 @f$ instead.
 * @param px A Vector of type @ref _avector "avector" that contains interpolation
 * points in x-direction.
 * @param py A Vector of type @ref _avector "avector" that contains interpolation
 * points in y-direction.
 * @param pz A Vector of type @ref _avector "avector" that contains interpolation
 * points in z-direction.
 * @param bem BEM-object containing additional information for computation
 * of the matrix entries.
 * @param V Matrix to store the computed results as stated above.
 */
HEADER_PREFIX void assemble_bem3d_dn_lagrange_linear_amatrix(const uint *idx,
    pcavector px, pcavector py, pcavector pz, pcbem3d bem, pamatrix V);

/**
 * @brief This function will evaluate the Lagrange polynomials in a given point
 * set <tt>X</tt> and store the results in a matrix <tt>V</tt>.
 *
 * The matrix entries of <tt>V</tt> will be computed as
 * @f[
 * \left( V \right)_{i\mu} :=  \mathcal L_\mu (\vec x_i)
 * @f]
 * with Lagrange polynomial @f$ \mathcal L_\mu (\vec x) @f$ defined as:
 * @f[
 * \mathcal L_\mu (\vec x) := \left( \prod_{\nu_1 = 1, \nu_1 \neq \mu_1}^m
 * \frac{\xi_{\nu_1} - x_1}{\xi_{\nu_1} - \xi_{\mu_1}} \right) \cdot
 * \left( \prod_{\nu_2 = 1, \nu_2 \neq \mu_2}^m
 * \frac{\xi_{\nu_2} - x_2}{\xi_{\nu_2} - \xi_{\mu_2}} \right) \cdot
 * \left( \prod_{\nu_3 = 1, \nu_3 \neq \mu_3}^m
 * \frac{\xi_{\nu_3} - x_3}{\xi_{\nu_3} - \xi_{\mu_3}} \right)
 * @f]
 *
 * @param X An array of 3D-vectors. <tt>X[i][0]</tt> will be the first
 * component of the i-th vector. Analogously <tt>X[i][1]</tt> will be the second
 * component of the i-th vector. The length of this array is determined by
 * <tt>V->cols</tt> .
 * @param px A Vector of type @ref _avector "avector" that contains interpolation
 * points in x-direction.
 * @param py A Vector of type @ref _avector "avector" that contains interpolation
 * points in y-direction.
 * @param pz A Vector of type @ref _avector "avector" that contains interpolation
 * points in z-direction.
 * @param V Matrix to store the computed results as stated above.
 */
HEADER_PREFIX void assemble_bem3d_lagrange_amatrix(const real (*X)[3],
    pcavector px, pcavector py, pcavector pz, pamatrix V);

/* ------------------------------------------------------------
 some useful functions
 ------------------------------------------------------------ */

/**
 * @brief Computes the @f$ L_2 @f$-projection of a given function using piecewise
 * constant basis functions.
 *
 * In case of piecewise constant basis functions the @f$ L_2 @f$-projection of a
 * function @f$ r @f$ is defined as:
 * @f[
 * f_i := \frac{2}{\lvert \Gamma_i \rvert} \, \int_\Gamma \, \varphi_i(\vec x)
 * \, r(\vec x, n(\vec x) ) \, \mathrm d \vec x \, ,
 * @f]
 * with @f$ \lvert \Gamma_i \rvert @f$ meaning the area of triangle @f$ i @f$
 * and @f$ n(\vec x) @f$ being the outpointing normal vector at the point
 * @f$ \vec x @f$.
 *
 * @param bem BEM-object containing additional information for computation
 * of the vector entries.
 * @param rhs This callback defines the function to be @f$ L_2 @f$ projected. Its
 * arguments are an evaluation 3D-vector <tt>x</tt> and its normal vector <tt>n</tt>.
 * @param f The @f$ L_2 @f$-projection coefficients are stored within this vector.
 * Therefore its length has to be at least <tt>bem->gr->triangles</tt>.
 */
HEADER_PREFIX void projectl2_bem3d_const_avector(pbem3d bem,
    boundary_func3d rhs, pavector f);

/**
 * @brief Computes the @f$ L_2 @f$-projection of a given function using linear
 * basis functions.
 *
 * In case of linear basis functions the @f$ L_2 @f$-projection of a
 * function @f$ r @f$ is defined as:
 * @f[
 * v_i := \int_\Gamma \, \varphi_i(\vec x)
 * \, r(\vec x, n(\vec x) ) \, \mathrm d \vec x \, ,
 * @f]
 * and then solving the equation
 * @f[
 * M \, f = v
 * @f]
 * with @f$ \lvert \Gamma_i \rvert @f$ meaning the area of triangle @f$ i @f$,
 * @f$ n(\vec x) @f$ being the outpointing normal vector at the point
 * @f$ \vec x @f$ and @f$ M @f$ being the mass matrix for linear basis functions
 * defined as:
 * @f[
 * M_{ij} := \int_\Gamma \varphi_i(\vec x) \, \varphi_j(\vec x) \, \mathrm d
 * \vec x \, .
 * @f]
 *
 * @param bem BEM-object containing additional information for computation
 * of the vector entries.
 * @param rhs This callback defines the function to be @f$ L_2 @f$ projected. Its
 * arguments are an evaluation 3D-vector <tt>x</tt> and its normal vector <tt>n</tt>.
 * @param f The @f$ L_2 @f$-projection coefficients are stored within this vector.
 * Therefore its length has to be at least <tt>bem->gr->triangles</tt>.
 */
HEADER_PREFIX void projectl2_bem3d_linear_avector(pbem3d bem,
    boundary_func3d rhs, pavector f);

/**
 * @brief initializes the field <tt>bem->v2t</tt> when using linear basis
 * functions
 *
 * @param bem the @ref _bem3d "bem3d" object for which the vertex to triangle map
 * should be computed.
 */
HEADER_PREFIX void setup_vertex_to_triangle_map_bem3d(pbem3d bem);

/**
 * @brief Generating quadrature points, weights and normal vectors on a cube
 * parameterization.
 *
 * This function will build quadrature points, weight and appropriate outpointing
 * normal vectors on a cube parameterization. The quadrature points are
 * stored in parameter <tt>Z</tt>, for which storage will be allocated, the normal
 * vectors and weights are stored within <tt>N</tt> and also storage will be
 * allocated from this function.
 *
 * @param bem BEM-object containing additional information for computation
 * of the quadrature points, weight and normal vectors.
 * @param a Minimal extent of the bounding box the parameterization will be
 * constructed around.
 * @param b Maximal extent of the bounding box the parameterization will be
 * constructed around.
 * @param delta Relative distance in terms of @f$ \operatorname{diam}(B_t) @f$
 * the parameterization is afar from the bounding box @f$ B_t @f$.
 * @param Z Resulting quadrature Points on the parameterization as array
 * of 3D-vectors. <tt>Z[i][0]</tt> will be the first
 * component of the i-th vector. Analogously <tt>Z[i][1]</tt> will be the second
 * component of the i-th vector. The length of this array is defined within the
 * function itself and the user has to know the length on his own.
 * @param N Resulting quadrature weight and normal vectors on the
 * parameterization as array of 3D-vectors. <tt>N[i][0]</tt> will be the first
 * component of the i-th vector. Analogously <tt>N[i][1]</tt> will be the second
 * component of the i-th vector. The length of this array is defined within the
 * function itself and the user has to know the length on his own.
 * Vectors do not have unit length instead it holds:
 * @f[
 * \lVert \vec n_i \rVert = \omega_i
 * @f]
 */
HEADER_PREFIX
void build_bem3d_cube_quadpoints(pcbem3d bem, const real a[3], const real b[3],
    const real delta, real (**Z)[3], real (**N)[3]);

/**
 * @brief For a block defined by <tt>row</tt> and <tt>col</tt> this function
 * will build a rank-k-approximation and return the @ref _rkmatrix "rkmatrix".
 *
 * This function will simply create a new @ref _rkmatrix "rkmatrix" and call
 * <tt>((pcbem3d)data)->farfield_rk</tt> to create the rank-k-approximation of
 * this block.
 *
 * @attention Before calling this function take care of initializing the @ref _bem3d
 * "bem3d" object with one the @ref _hmatrix "hmatrix" approximation techniques
 * such as @ref setup_hmatrix_aprx_inter_row_bem3d. Not initialized @ref _bem3d
 * "bem3d" object will cause undefined behavior.
 *
 * @param row Row @ref _cluster "cluster" defining the current @ref _hmatrix
 * "hmatrix" block.
 * @param col Column @ref _cluster "cluster" defining the current @ref _hmatrix
 * "hmatrix" block.
 * @param data This object has to be a valid and initialized @ref _bem3d "bem3d"
 * object.
 *
 * @return A rank-k-approximation of the current block is returned.
 */
HEADER_PREFIX prkmatrix build_bem3d_rkmatrix(pccluster row, pccluster col,
    void* data);

/**
 * @brief For a block defined by <tt>row</tt> and <tt>col</tt> this function
 * will compute the full submatrix and return an @ref _amatrix "amatrix".
 *
 * This function will simply create a new @ref _amatrix "amatrix" and call
 * <tt>((pcbem3d)data)->nearfield</tt> to compute the submatrix for this block.
 *
 * @param row Row @ref _cluster "cluster" defining the current @ref _hmatrix
 * "hmatrix" block.
 * @param col Column @ref _cluster "cluster" defining the current @ref _hmatrix
 * "hmatrix" block.
 * @param data This object has to be a valid @ref _bem3d "bem3d"
 * object.
 *
 * @return Submatrix @f$ G_{|t \times s} @f$.
 */
HEADER_PREFIX pamatrix build_bem3d_amatrix(pccluster row, pccluster col,
    void* data);

/** @} */

#endif /* BEM3D_H_ */

