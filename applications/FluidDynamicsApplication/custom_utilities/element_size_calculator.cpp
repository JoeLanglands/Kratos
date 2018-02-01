#include "element_size_calculator.h"

namespace Kratos {

template< std::size_t TDim, std::size_t TNumNodes >
ElementSizeCalculator<TDim,TNumNodes>::~ElementSizeCalculator() {};

template<>
double ElementSizeCalculator<2,3>::MinimumElementSize(const Geometry<Node<3> >& rGeometry) {

    /* Calculate node-edge distances */
    double x10 = rGeometry[1].X() - rGeometry[0].X();
    double y10 = rGeometry[1].Y() - rGeometry[0].Y();
    double x20 = rGeometry[2].X() - rGeometry[0].X();
    double y20 = rGeometry[2].Y() - rGeometry[0].Y();

    // node 0, edge 12
    double nx = -(y20-y10);
    double ny = x20-x10;
    double Hsq = x10*nx + y10*ny;
    Hsq *= Hsq / (nx*nx + ny*ny);

    // node 1, edge 20
    nx = -y20;
    ny = x20;
    double hsq = x10*nx + y10*ny;
    hsq *= hsq / (nx*nx + ny*ny);
    Hsq = ( hsq < Hsq ) ? hsq : Hsq;

    // node 2, edge 10
    nx = -y10;
    ny = x10;
    hsq = x20*nx + y20*ny;
    hsq *= hsq / (nx*nx + ny*ny);
    Hsq = ( hsq < Hsq ) ? hsq : Hsq;

    return std::sqrt(Hsq);
}

template<>
double ElementSizeCalculator<2,4>::MinimumElementSize(const Geometry<Node<3> >& rGeometry) {
    
    /* Calculate node-edge distances, assuming parallel faces */
    double x10 = rGeometry[1].X() - rGeometry[0].X();
    double y10 = rGeometry[1].Y() - rGeometry[0].Y();
    double x20 = rGeometry[2].X() - rGeometry[0].X();
    double y20 = rGeometry[2].Y() - rGeometry[0].Y();
    double x30 = rGeometry[3].X() - rGeometry[0].X();
    double y30 = rGeometry[3].Y() - rGeometry[0].Y();

    // node 0, edge 12
    double nx = -(y20-y10);
    double ny = x20-y10;
    double Hsq = x10*nx + y10*ny;
    Hsq *= Hsq / std::sqrt(nx*nx + ny*ny);

    // node 0, edge 23
    nx = -(y30-y20);
    ny = x30-y20;
    double hsq = x20*nx + y20*ny;
    hsq *= hsq / (nx*nx + ny*ny);
    Hsq = ( hsq < Hsq ) ? hsq : Hsq;

    return std::sqrt(Hsq);
}

template<>
double ElementSizeCalculator<3,4>::MinimumElementSize(const Geometry<Node<3> >& rGeometry) {

    /* Calculate distances between each node and the opposite face */
    double x10 = rGeometry[1].X() - rGeometry[0].X();
    double y10 = rGeometry[1].Y() - rGeometry[0].Y();
    double z10 = rGeometry[1].Z() - rGeometry[0].Z();

    double x20 = rGeometry[2].X() - rGeometry[0].X();
    double y20 = rGeometry[2].Y() - rGeometry[0].Y();
    double z20 = rGeometry[2].Z() - rGeometry[0].Z();

    double x30 = rGeometry[3].X() - rGeometry[0].X();
    double y30 = rGeometry[3].Y() - rGeometry[0].Y();
    double z30 = rGeometry[3].Z() - rGeometry[0].Z();

    // face 123
    double nx = (y30-y10)*(z20-z10) - (z30-z10)*(y20-y10);
    double ny = (z30-z10)*(x20-x10) - (x30-x10)*(z20-z10);
    double nz = (x30-x10)*(y20-y10) - (y30-y10)*(x20-x10);
    double Hsq = x10*nx + y10*ny + z10*nz; // scalar product x10*n
    Hsq *= Hsq / (nx*nx + ny*ny + nz*nz); // H^2 = (x10*n)^2 / ||n||^2

    // face 230
    nx = y30*z20 - z30*y20;
    ny = z30*x20 - x30*z20;
    nz = x30*y20 - y30*x20;
    double hsq = x10*nx + y10*ny + z10*nz;
    hsq *= hsq / (nx*nx + ny*ny + nz*nz);
    Hsq = (hsq < Hsq) ? hsq : Hsq;

    // face 301
    nx = y10*z30 - z10*y30;
    ny = z10*x30 - x10*z30;
    nz = x10*y30 - y10*x30;
    hsq = x20*nx + y20*ny + z20*nz;
    hsq *= hsq / (nx*nx + ny*ny + nz*nz);
    Hsq = (hsq < Hsq) ? hsq : Hsq;

    // face 012
    nx = y10*z20 - z10*y20;
    ny = z10*x20 - x10*z20;
    nz = x10*y20 - y10*x20;
    hsq = x30*nx + y30*ny + z30*nz;
    hsq *= hsq / (nx*nx + ny*ny + nz*nz);
    Hsq = (hsq < Hsq) ? hsq : Hsq;
    return std::sqrt(Hsq);
}

template<>
double ElementSizeCalculator<3,8>::MinimumElementSize(const Geometry<Node<3> >& rGeometry) {

    /* Numbering assumes bottom face nodes 0123, top nodes 4567
     * considering the distance between a few face--node pairs:
     * face node
     *  034  1
     *  014  3
     *  013  4
     * This assumes parallel faces!
     * (otherwise all nodes should be checked against opposite faces)
     */
    double x10 = rGeometry[1].X() - rGeometry[0].X();
    double y10 = rGeometry[1].Y() - rGeometry[0].Y();
    double z10 = rGeometry[1].Z() - rGeometry[0].Z();

    double x30 = rGeometry[3].X() - rGeometry[0].X();
    double y30 = rGeometry[3].Y() - rGeometry[0].Y();
    double z30 = rGeometry[3].Z() - rGeometry[0].Z();

    double x40 = rGeometry[4].X() - rGeometry[0].X();
    double y40 = rGeometry[4].Y() - rGeometry[0].Y();
    double z40 = rGeometry[4].Z() - rGeometry[0].Z();

    // Face 034
    double nx = y30*z40 - z30*y40;
    double ny = z30*x40 - x30*z40;
    double nz = x30*y40 - y30*x40;
    double Hsq = x10*nx + y10*ny + z10*nz; // scalar product x10*n
    Hsq *= Hsq / (nx*nx + ny*ny + nz*nz); // H^2 = (x10*n)^2 / ||n||^2

    // face 014
    nx = y10*z40 - z10*y40;
    ny = z10*x40 - x10*z40;
    nz = x10*y40 - y10*x40;
    double hsq = x30*nx + y30*ny + z30*nz;
    hsq *= hsq / (nx*nx + ny*ny + nz*nz);
    Hsq = (hsq < Hsq) ? hsq : Hsq;

    // face 013
    nx = y10*z30 - z10*y30;
    ny = z10*x30 - x10*z30;
    nz = x10*y30 - y10*x30;
    hsq = x40*nx + y40*ny + z40*nz;
    hsq *= hsq / (nx*nx + ny*ny + nz*nz);
    Hsq = (hsq < Hsq) ? hsq : Hsq;
    return std::sqrt(Hsq);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Template class instantiation

template class ElementSizeCalculator<2,3>;
template class ElementSizeCalculator<2,4>;
template class ElementSizeCalculator<3,4>;
template class ElementSizeCalculator<3,8>;

///////////////////////////////////////////////////////////////////////////////////////////////////

}