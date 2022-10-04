#pragma once
class CW
{
public:
    int T; // document id
    int l; // left boundary
    int c; // center
    int r; // right boundary

    CW() {}

    CW(const int &_T, const int &_l, const int &_c, const int &_r) : T(_T), l(_l), c(_c), r(_r) {}

    bool operator< (const CW & tmp)const{
        return T < tmp.T;
    }

    void display() const
    {
        printf("(document id: %d, l: %d, c: %d, r: %d)\n", T, l, c, r);
    }
};

class Wrapped_CW{
    public:
    int token_id;
    int ith_hash;
    CW cw;
    
    Wrapped_CW(){}

    Wrapped_CW(const int & _token_id, const int& _ith_hash, const int& _T, const int &_l, const int &_c, const int &_r):
        token_id(_token_id), ith_hash(_ith_hash),cw(_T, _l, _c, _r){}

    bool operator< (const Wrapped_CW & tmp)const{
        if(token_id == tmp.token_id){
            if(ith_hash == tmp.ith_hash){
                return cw<tmp.cw;
            }
            return ith_hash < tmp.ith_hash;
        }
        return token_id < tmp.token_id;
    }

    void display(){
         printf("token_id:%d, ith_hashs: %d\n", token_id, ith_hash);
    }
};