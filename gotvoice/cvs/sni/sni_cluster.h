//
//	sni_cluster.h
//

//
//	the aim is to find the set of cluster centers k, by vector quantization
//
class CDMap
{
public:
	inline CDMap()
	{
		i =0;
		d = 0.0;
		pnext = plast = NULL;
	}

	inline ~CDMap(){};

	int i;
	double d;
	CDMap *pnext, *plast;
};

class CCluster
{
public:
	inline CCluster()
	{
		pFirstMap = pLastMap = NULL;
		nMap = 0;
	};

	inline ~CCluster()
	{
		if( pFirstMap )
		{
			CDMap *pm, *pmm;

			pm = pFirstMap;
			pmm = pFirstMap->pnext;

			for(; pm; pm = pmm)
			{
				pmm = pm->pnext;

				delete pm;
			}
		}
	};

	CDMap *pFirstMap, *pLastMap;
	int nMap;
};
