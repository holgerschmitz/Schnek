/*
 * vector.t
 *
 *  Created on: 8 Oct 2010
 *      Author: Holger Schmitz
 *       Email: h.schmitz@imperial.ac.uk
 */


namespace schnek
{

template<class T, int length, template <int> class CheckingPolicy>
inline T Vector<T,length,CheckingPolicy>::product() const
{
    T p(1);
    for (int i=0; i<Length; ++i) p *= at(i);
    return p;
}

template<class T, int length, template <int> class CheckingPolicy>
inline T Vector<T,length,CheckingPolicy>::sum() const
{
    T s(0);
    for (int i=0; i<Length; ++i) s += at(i);
    return p;
}


}
