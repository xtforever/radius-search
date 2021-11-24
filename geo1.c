#include "mls.h"
#include <math.h>
#include "m_tool.h"

/* x=lon/ y=lat */
typedef struct geopt {
    int plz;
    char ort[100];
    double lon,lat;
} geopt_t;

int PLZ;

int MATRIX;
geopt_t MA_START;
void check_all(void);
void init_matrix(void)
{               
    PLZ=m_create(10000,sizeof(int) );
    m_setlen(PLZ,10000);
    MATRIX=m_create(100,sizeof(int) );
    m_setlen(MATRIX,100);
    check_all();
    MA_START.lon = 15;
    MA_START.lat = 55.1;
}

typedef struct plz_st {
    int plz;
    double lon,lat;
} plz_t;


void insert_plz( int plz, double lon, double lat )
{
    int x = plz / 10; /* 0-9999 simple hashing*/
    int lst = INT( PLZ, x );
    if( lst == 0 )  lst = INT( PLZ, x ) = m_create(1,sizeof(plz_t));
    int p; plz_t *d;
    m_foreach(lst,p,d) {
	if( d->plz == plz ) 
	    return;
    }
    d=m_add(lst);
    d->lon=lon;
    d->lat=lat;
    d->plz=plz;
}

int get_geopos( int plz, double *lon, double *lat )
{
    int x = plz / 10; /* 0-9999 */
    int lst = INT( PLZ, x );
    int p; plz_t *d;
    m_foreach(lst,p,d) {
	if( d->plz == plz ) {
	    *lon = d->lon;
	    *lat = d->lat;
	    return 0;
	}
    }
    printf("plz nicht gefunden\n");
    return -1;
}

void geo_distance( geopt_t *a,  geopt_t  *b, double *dx, double *dy );


void insert_above(int num)
{
    TRACE(2,"matrix move dy=%d", num );
    m_ins( MATRIX, 0, num );
}

void insert_left(int num)
{
    TRACE(2,"matrix move dx=%d", num );
    int p,*line;
    m_foreach( MATRIX, p, line ) {
	if( *line ) {
	    m_ins( *line,0, num );
	}
    }
}


void check_all()
{
    int p,*d;
    m_foreach(MATRIX,p,d) {

	if(! *d ) continue;
	int cell,*cellp;
	m_setlen( *d, m_bufsize(*d) );
	
	m_foreach(*d, cell, cellp ) {
	    if( *cellp ) {
		m_len(*cellp);
	    }
	}
    }	
}

void insert_geopt( geopt_t *a )
{
    double dx,dy;
    geo_distance( &MA_START, a, &dx, &dy );
    TRACE(2, "%.2lf / %.2lf : distance= x=%.2lf / y=%.2lf", a->lon, a->lat, dx,dy );
    int x,y;
    x=dx;
    y=dy;
    if( x < 0 ) {
	MA_START.lon = a->lon;
	insert_left( -x );
	x=0;
    };
    
    if( y < 0 ) {
	MA_START.lat = a->lat;
	insert_above( -y );
	y=0;
    };
    // zeile
    if( m_len(MATRIX) <= y ) {
	m_setlen(MATRIX,y+1);
    }
    int m = INT(MATRIX,y);

    // spalte
    if( !m ) {
	m=INT(MATRIX,y)=m_create(x+1,sizeof(int));
	m_setlen( m, x+1 );
    }

    if( m_len(m) <= x ) {
	m_setlen(m, x+1 );
    }

    // element
    int pt_list = INT( m,x );
    if( !pt_list ) {
	pt_list =  INT( m,x ) = m_create(1,sizeof(geopt_t));
    }
    
    m_put( pt_list, a );

    TRACE(2, "new element at %d / %d", x,y );
    TRACE(2, "left/top = %.2lf / %.2lf",MA_START.lon,MA_START.lat  );
}


/* abstand in kilometer x,y */
void geo_distance( geopt_t *a,  geopt_t  *b, double *dx, double *dy )
{
    // 1° = π/180 rad ≈ 0.01745
    //
    double lat;
    
    lat = (a->lat + b->lat) / 2 * 0.01745;
    *dx = 111.3 * cos(lat) * (a->lon - b->lon);
    *dy = 111.3 * (a->lat - b->lat); 
}

double geo_distance_abs( geopt_t *a,  geopt_t  *b )
{
    double x,y;
    geo_distance( a,b,&x,&y);
    return sqrt( x*x + y*y );
}


int get_ptl( int x, int y )
{
    if( x>= 0 && y >= 0 ) {
	if( y < m_len(MATRIX) ) {
	    int ln = INT(MATRIX,y);
	    if( ln && x < m_len(ln) ) {
		int ptl = INT(ln,x);
		return ptl; 
	    }
	}	    
    }
    return 0;
}


int find_geopt( geopt_t *a )
{
    double dx,dy;
    geo_distance( &MA_START, a, &dx, &dy );
    TRACE(2, "x=%lf / y=%lf\n", dx,dy );
    int x,y;
    x=dx;
    y=dy;
    
    if( x>= 0 && y >= 0 ) {
	if( y < m_len(MATRIX) ) {
	    int ln = INT(MATRIX,y);

	    if( x < m_len(ln) ) {
		int ptl = INT(ln,x);
		if( ptl ) {
		    return ptl; 
		}
	    }	    
	}	
    }

    WARN("ausserhalb %lf / %lf", dx,dy );
    return 0;
}


void dump_pt(geopt_t *a)
{
    printf("%d %.2lf %.2lf %s\n",
	   a->plz, a->lon, a->lat, a->ort );
}

void dump_ptl(int ptl)
{
    if( ptl <= 0 ) {
	printf("keine orte gefunden\n");
	return;
    }
    int p; geopt_t *d;
    m_foreach( ptl,p,d) {
	dump_pt(d);
    }
}

//                      Breite/lat/grad		Laenge/lon/grad
// Rüsselsheim Bahnhof 	49.9917			8.41321
// Dezimalgrad 
//
geopt_t ex[] = {
    { 1067, "Dresden", 13.7210676148814,51.060033646337 },
    { 1169, "Dresden", 13.669187770824,51.040255399397 },
    { 1616, "Strehla", 13.233513371901,51.362777454525 },
    { 99991, "Großengottern", 10.5480003363303, 51.1797540266442 },
    { 26603, "Aurich",  7.48612800531259,        53.4742866866241 },
    { 97947, "Grünsfeld",   9.76227550109563,       49.6174289644197 }
};



int read_pts_from_file( char *filename );

void test1( void )
{
    trace_level=2;
    double dist;
    dist =  geo_distance_abs( ex, ex+2 );
    printf( "%lf\n", dist);


    // read_pts_from_file("plz2.txt");
    
    int max=ALEN(ex);
    for(int i=0; i<max; i++ ) {		
	insert_geopt( ex+i );
    }
    if ( find_geopt( ex ) == 4 )
	TRACE(2,"OK");
    else
	TRACE(2,"ERR");
    
    read_pts_from_file("plz2.txt");
    
    printf("%lf,%lf\n", MA_START.lon, MA_START.lat );
    
    for(int i=0; i<max; i++ ) {	
	int l = find_geopt( ex+i );
	TRACE(2,"dump %d",l);
	dump_ptl(l);
    }



    

}


void store_pt(int  buf)
{
    geopt_t pt;
    pt.plz = atoi( STR(buf,0) );
    strncpy( pt.ort,STR(buf,1), sizeof(pt.ort) );
    pt.lon =  atof(STR(buf,2 ));
    pt.lat =  atof(STR(buf,3 ));
    insert_geopt(&pt);
    insert_plz( pt.plz, pt.lon, pt.lat );
}

// PLZ     Ort     Lon     LAT
int read_pts_from_file( char *filename )
{
    FILE *fp = fopen( filename, "r" );
    ASSERT(fp);
    int cnt=0;
    int sbuf=0;
    int buf = m_create(100,1);
    while( m_fscan(buf,'\n', fp ) == '\n' ) {
	cnt++;
	sbuf =  m_split(sbuf,m_str(buf),9, 1 );
	if( m_len(sbuf) == 4 )
	    store_pt(sbuf);
	m_free_strings(sbuf,1);
	m_clear(buf);

	
    }
    fclose(fp);
    m_free(buf);
    m_free(sbuf);
    TRACE(3,"Read Lines: %d", cnt);
    return 0;
}

void find_radius(double dx, double dy, int dist )
{
    int ptl = m_create( 10, sizeof( geopt_t) );

    geopt_t a;
    a.lon = dx;
    a.lat = dy;
    geo_distance( &MA_START, & a, &dx, &dy );
    int x,y,x0,y0;
    x=dx;
    y=dy;
    
    for(x0= Max(0,x - dist); x0 <= x + dist; x0++ )
	{
	    for( y0 = Max(0,y - dist); y0 <= y+dist; y0++ )
		{
		    TRACE(2,"inspect %d / %d", x0, y0 );
		    int p = get_ptl(x0,y0);
		    m_concat( ptl, p );
		}
	}
    if(! ptl ) return;
    
    int p; geopt_t *d; int ax;
    m_foreach( ptl,p,d) {
	if( (ax=geo_distance_abs( &a,d )) <= dist ) 
	    printf("%d %d %.2lf %.2lf %s\n",
		   ax,
		   d->plz, d->lon, d->lat, d->ort );
    }
    
    m_free(ptl);
}



int main(int argc, char **argv )
{
    double lon, lat; int dist,plz;
    int e;
    char cmd;
    FILE *fp = stdin;

    m_init();
    trace_level=3;
    init_matrix();
    read_pts_from_file("plz2.txt");
    
    int buf = m_create(10,1);
    while( m_fscan(buf,'\n', fp ) == '\n' ) {
	if( m_len(buf) < 2 ) continue;
	if( CHAR(buf,0) == 'r' ) {	
	    e = sscanf(m_str(buf), "%c %lf %lf %d", &cmd, &lon, &lat, &dist );
	    if( e==4 ) find_radius( lon,lat,dist );
	} else if( CHAR(buf,0) == 'p' ) {
	    e = sscanf(m_str(buf), "%c %d",&cmd, &plz );
	    if(e==2 && get_geopos( plz, &lon, &lat )==0 ) {
		printf("%lf %lf\n", lon, lat );
	    }
	} else if( CHAR(buf,0) == 'u' ) {	
	    e = sscanf(m_str(buf), "%c %d %d", &cmd, &plz, &dist );
	    if( e==3 && get_geopos( plz, &lon, &lat )==0 ) {
		find_radius( lon,lat,dist );
	    }
	}
	m_clear(buf);
	fflush(stdout);
    }
    m_free(buf);
        
    { int p,*d; m_foreach(MATRIX,p,d) { m_free_list(*d); }; m_free(MATRIX); }
    m_free_list(PLZ);
    m_destruct();
    return 0;
}
 
