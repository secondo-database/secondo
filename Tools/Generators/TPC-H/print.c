/* @(#)print.c	2.1.8.2 */
/* generate flat files for data load */
#include <stdio.h>
#ifndef VMS
#include <sys/types.h>
#endif
#if defined(SUN)
#include <unistd.h>
#endif
#include <math.h>

#include "dss.h"
#include "dsstypes.h"
#include <string.h>

/* M. Spiekermann. Global variable used to detect the last record. */
extern int GLOB_LAST_RECORD;

/*
 * Function Prototypes
 */
FILE *print_prep PROTO((int table, int update));
int pr_drange PROTO((int tbl, long min, long cnt, long num));

FILE *
print_prep(int table, int update)
{
	char upath[128];
	FILE *res;

	if (updates)
		{
		if (update > 0) /* updates */
			if ( insert_segments )
				{
				int this_segment;
				if(strcmp(tdefs[table].name,"orders_tbl"))
					this_segment=++insert_orders_segment;
				else 
					this_segment=++insert_lineitem_segment;
				sprintf(upath, "%s%c%s.u%d.%d", 
					env_config(PATH_TAG, PATH_DFLT),
					PATH_SEP, tdefs[table].name, update%10000,this_segment);
				}
			else
				{
				sprintf(upath, "%s%c%s.u%d",
				env_config(PATH_TAG, PATH_DFLT),
				PATH_SEP, tdefs[table].name, update);
				}
		else /* deletes */
			if ( delete_segments )
				{
				++delete_segment;
				sprintf(upath, "%s%cdelete.u%d.%d",
					env_config(PATH_TAG, PATH_DFLT), PATH_SEP, -update%10000,
					delete_segment);
				}
			else
				{
				sprintf(upath, "%s%cdelete.%d",
				env_config(PATH_TAG, PATH_DFLT), PATH_SEP, -update);
				}
		return(fopen(upath, "w"));
        }
    res = tbl_open(table, "w");
    OPEN_CHECK(res, tdefs[table].name);
    return(res);
}

int
dbg_print(int format, FILE *target, void *data, int len, int sep)
{
	int dollars,
		cents;

	switch(format)
	{
	case DT_STR:
		if (columnar) {
		    fprintf(target, "%-*s", len, (char *)data);
		} else {
		  if (sep >= 2 ) // 2 and 3 are used to indicate text atoms
		    fprintf(target, "<text>%s</text--->", (char *)data);
		  else
		    fprintf(target, "\"%s\"", (char *)data);
		}	
		break;
#ifdef MVS
	case DT_VSTR:
		/* note: only used in MVS, assumes columnar output */
		fprintf(target, "%c%c%-*s", 
			(len >> 8) & 0xFF, len & 0xFF, len, (char *)data);
		break;
#endif /* MVS */
	case DT_INT:
		if (columnar)
			fprintf(target, "%12ld", (long)data);
		else
			fprintf(target, "%ld", (long)data);
		break;
	case DT_HUGE:
#ifndef SUPPORT_64BITS
        if (*(long *)((long *)data + 1) == 0) \
           if (columnar) fprintf(target, "%12ld", *(long *)data);
           else fprintf(target, "%ld", *(long *)data);
        else
           if (columnar) fprintf(target, "%5ld%07ld", 
				*(long *)((long *)data + 1), *(long *)data);
           else fprintf(target,"%ld%07ld", 
				*(long *)((long *)data + 1), *(long *)data);
#else
		fprintf(target, HUGE_FORMAT, *(DSS_HUGE *)data);
#endif /* SUPPORT_64BITS */
		break;
	case DT_KEY:
		fprintf(target, "%ld", (long)data);
		break;
	case DT_MONEY:
		cents = (long)data;
		if (cents < 0)
			{
			fprintf(target, "-");
			cents = -cents;
			}
		dollars = cents / 100;
		cents %= 100;
		if (columnar)
			fprintf(target, "%12ld.%02ld", dollars, cents);
		else
			fprintf(target, "%ld.%02ld", dollars, cents);
		break;
	case DT_CHR:
		if (columnar)
			fprintf(target, "%c ", ((char*)data)[0]);
		else
			fprintf(target, "\"%c\"", ((char*)data)[0]);
		break;
	}

#ifdef EOL_HANDLING
	if (sep)
#endif /* EOL_HANDLING */
	if (!columnar && (sep != -1) && (sep != 3) )
		fprintf(target, "%c", SEPARATOR);
	
	return(0);
}

int
pr_cust(customer_t *c, int mode)
{
static FILE *fp = NULL;
        
   if (fp == NULL) {
     fp = print_prep(CUST, 0);
     fprintf(fp, "(OBJECT CUSTOMER \n");
     fprintf(fp, "  ()");
     fprintf(fp, "  (rel( tuple ( (C_CUSTKEY int) \n");
     fprintf(fp, "                (C_NAME string) \n");
     fprintf(fp, "                (C_ADDRESS string) \n");
     fprintf(fp, "                (C_NATIONKEY int) \n");
     fprintf(fp, "                (C_PHONE string) \n");
     fprintf(fp, "                (C_ACCTBAL real) \n");
     fprintf(fp, "                (C_MKTSEGMENT string) \n");
     fprintf(fp, "                (C_COMMENT text) )))\n");
     fprintf(fp, "  ( \n");
   }

   PR_STRT(fp);
   PR_INT(fp, c->custkey);
   PR_VSTR(fp, c->name, C_NAME_LEN);
   PR_VSTR(fp, c->address, 
       (columnar)?(long)(ceil(C_ADDR_LEN * V_STR_HGH)):c->alen);
   PR_INT(fp, c->nation_code);
   PR_STR(fp, c->phone, PHONE_LEN);
   PR_MONEY(fp, c->acctbal);
   PR_STR(fp, c->mktsegment, C_MSEG_LEN);
   PR_TEXT(fp, c->comment, 
       (columnar)?(long)(ceil(C_CMNT_LEN * V_STR_HGH)):c->clen);
   PR_END(fp,GLOB_LAST_RECORD);

   return(0);
}

/*
 * print the numbered order 
 */
int
pr_order(order_t *o, int mode)
{
    static FILE *fp_o = NULL;
    static int last_mode = 0;
        
    if (fp_o == NULL || mode != last_mode)
    {
        if (fp_o) {
            fclose(fp_o);
	}
        fp_o = print_prep(ORDER, mode);
        last_mode = mode;
	
     fprintf(fp_o, "(OBJECT ORDERS \n");
     fprintf(fp_o, "  ()");
     fprintf(fp_o, "  (rel( tuple ( (O_OERDERKEY int) \n");
     fprintf(fp_o, "                (O_CUSTKEY int) \n");
     fprintf(fp_o, "                (O_ORDERSTATUS string) \n");
     fprintf(fp_o, "                (O_TOTALPRICE real) \n");
     fprintf(fp_o, "                (O_ORDERDATE instant) \n");
     fprintf(fp_o, "                (O_ORDERPRIORITY string) \n");
     fprintf(fp_o, "                (O_CLERK string) \n");
     fprintf(fp_o, "                (O_SHIPPRIORITY int) \n");     
     fprintf(fp_o, "                (O_COMMENT text) )))\n");
     fprintf(fp_o, "  ( \n");
	
    }
	
    PR_STRT(fp_o);
    PR_HUGE(fp_o, o->okey);
    PR_INT(fp_o, o->custkey);
    PR_CHR(fp_o, o->orderstatus);
    PR_MONEY(fp_o, o->totalprice);
    PR_STR(fp_o, o->odate, DATE_LEN);
    PR_STR(fp_o, o->opriority, O_OPRIO_LEN);
    PR_STR(fp_o, o->clerk, O_CLRK_LEN);
    PR_INT(fp_o, o->spriority);
    PR_TEXT_LAST(fp_o, o->comment, 
       (columnar)?(long)(ceil(O_CMNT_LEN * V_STR_HGH)):o->clen);
    PR_END(fp_o, GLOB_LAST_RECORD);

    return(0);
}

/*
 * print an order's lineitems
 */
int
pr_line(order_t *o, int mode)
{
    static FILE *fp_l = NULL;
    static int last_mode = 0;
    long      i;
        
    if (fp_l == NULL || mode != last_mode)
    {
        if (fp_l) 
            fclose(fp_l);
        fp_l = print_prep(LINE, mode);
        last_mode = mode;

     fprintf(fp_l, "(OBJECT LINEITEM \n");
     fprintf(fp_l, "  ()");
     fprintf(fp_l, "  (rel( tuple ( (L_OERDERKEY int) \n");
     fprintf(fp_l, "                (L_PARTKEY int) \n");
     fprintf(fp_l, "                (L_SUPPKEY int) \n");
     fprintf(fp_l, "                (L_LINENUMBER int) \n");
     fprintf(fp_l, "                (L_QUANTITY real) \n");
     fprintf(fp_l, "                (L_EXTENDENPRICE real) \n");
     fprintf(fp_l, "                (L_DISCOUNT real) \n");
     fprintf(fp_l, "                (L_TAX real) \n");
     fprintf(fp_l, "                (L_RETURNFLAG string) \n");
     fprintf(fp_l, "                (L_LINESTATUS string) \n");
     fprintf(fp_l, "                (L_SHIPDATE instant) \n");
     fprintf(fp_l, "                (L_COMMITDATE instant) \n"); 
     fprintf(fp_l, "                (L_RECEIPTDATE instant) \n");
     fprintf(fp_l, "                (L_SHIPINSTRUCT string) \n");
     fprintf(fp_l, "                (L_SHIPMODE string) \n");                        
     fprintf(fp_l, "                (L_COMMENT string) )))\n");
     fprintf(fp_l, "  ( \n");
	
	
    }

    for (i = 0; i < o->lines; i++)
        {
        PR_STRT(fp_l);
        PR_HUGE(fp_l, o->l[i].okey);
        PR_INT(fp_l, o->l[i].partkey);
        PR_INT(fp_l, o->l[i].suppkey);
        PR_INT(fp_l, o->l[i].lcnt);
        PR_INT(fp_l, o->l[i].quantity);
        PR_MONEY(fp_l, o->l[i].eprice);
        PR_MONEY(fp_l, o->l[i].discount);
        PR_MONEY(fp_l, o->l[i].tax);
        PR_CHR(fp_l, o->l[i].rflag[0]);
        PR_CHR(fp_l, o->l[i].lstatus[0]);
        PR_STR(fp_l, o->l[i].sdate, DATE_LEN);
        PR_STR(fp_l, o->l[i].cdate, DATE_LEN);
        PR_STR(fp_l, o->l[i].rdate, DATE_LEN);
        PR_STR(fp_l, o->l[i].shipinstruct, L_INST_LEN);
        PR_STR(fp_l, o->l[i].shipmode, L_SMODE_LEN);
        PR_VSTR_LAST(fp_l, o->l[i].comment, 
            (columnar)?(long)(ceil(L_CMNT_LEN * V_STR_HGH)):o->l[i].clen);
        PR_END(fp_l, GLOB_LAST_RECORD && (i == o->lines-1));
        }

   return(0);
}

/*
 * print the numbered order *and* its associated lineitems
 */
int
pr_order_line(order_t *o, int mode)
{
    tdefs[ORDER].name = tdefs[ORDER_LINE].name;
    pr_order(o, mode);
    pr_line(o, mode);

    return(0);
}

/*
 * print the given part
 */
int
pr_part(part_t *part, int mode)
{
static FILE *p_fp = NULL;

    if (p_fp == NULL)
    {
        p_fp = print_prep(PART, 0);

     fprintf(p_fp, "(OBJECT PART \n");
     fprintf(p_fp, "  ()");
     fprintf(p_fp, "  (rel( tuple ( (P_PARTKEY int) \n");
     fprintf(p_fp, "                (P_NAME text) \n");
     fprintf(p_fp, "                (P_MFGR string) \n");
     fprintf(p_fp, "                (P_BRAND string) \n");
     fprintf(p_fp, "                (P_TYPE string) \n");
     fprintf(p_fp, "                (P_SIZE int) \n");
     fprintf(p_fp, "                (P_CONTAINER string) \n");
     fprintf(p_fp, "                (P_RETAILPRICE real) \n");                       
     fprintf(p_fp, "                (P_COMMENT string) )))\n");
     fprintf(p_fp, "  ( \n");
   }

   PR_STRT(p_fp);
   PR_INT(p_fp, part->partkey);
   PR_TEXT(p_fp, part->name,
       (columnar)?(long)P_NAME_LEN:part->nlen);
   PR_STR(p_fp, part->mfgr, P_MFG_LEN);
   PR_STR(p_fp, part->brand, P_BRND_LEN);
   PR_VSTR(p_fp, part->type,
       (columnar)?(long)P_TYPE_LEN:part->tlen);
   PR_INT(p_fp, part->size);
   PR_STR(p_fp, part->container, P_CNTR_LEN);
   PR_MONEY(p_fp, part->retailprice);
   PR_VSTR_LAST(p_fp, part->comment, 
       (columnar)?(long)(ceil(P_CMNT_LEN * V_STR_HGH)):part->clen);
   PR_END(p_fp, GLOB_LAST_RECORD);

   return(0);
}

/*
 * print the given part's suppliers
 */
int
pr_psupp(part_t *part, int mode)
{
    static FILE *ps_fp = NULL;
    long      i;

    if (ps_fp == NULL)
    {
        ps_fp = print_prep(PSUPP, mode);
	
     fprintf(ps_fp, "(OBJECT PARTSUPP \n");
     fprintf(ps_fp, "  ()");
     fprintf(ps_fp, "  (rel( tuple ( (PS_PARTKEY int) \n");
     fprintf(ps_fp, "                (PS_SUPPKEY int) \n");
     fprintf(ps_fp, "                (PS_AVAILQTY int) \n");
     fprintf(ps_fp, "                (PS_SUPPLYCOST real) \n");                 
     fprintf(ps_fp, "                (PS_COMMENT text) )))\n");
     fprintf(ps_fp, "  ( \n");
   }


   for (i = 0; i < SUPP_PER_PART; i++)
      {
      PR_STRT(ps_fp);
      PR_INT(ps_fp, part->s[i].partkey);
      PR_INT(ps_fp, part->s[i].suppkey);
      PR_INT(ps_fp, part->s[i].qty);
      PR_MONEY(ps_fp, part->s[i].scost);
      PR_TEXT_LAST(ps_fp, part->s[i].comment, 
       (columnar)?(long)(ceil(PS_CMNT_LEN * V_STR_HGH)):part->s[i].clen);
      PR_END(ps_fp, GLOB_LAST_RECORD && ( i == SUPP_PER_PART-1) );
      }

   return(0);
}

/*
 * print the given part *and* its suppliers
 */
int
pr_part_psupp(part_t *part, int mode)
{
    tdefs[PART].name = tdefs[PART_PSUPP].name;
    pr_part(part, mode);
    pr_psupp(part, mode);

    return(0);
}

int
pr_supp(supplier_t *supp, int mode)
{
static FILE *fp = NULL;
        
   if (fp == NULL)
   {
        fp = print_prep(SUPP, mode);

     fprintf(fp, "(OBJECT SUPPLIER \n");
     fprintf(fp, "  ()");
     fprintf(fp, "  (rel( tuple ( (S_SUPPKEY int) \n");
     fprintf(fp, "                (S_NAME string) \n");
     fprintf(fp, "                (S_ADDRESS string) \n");
     fprintf(fp, "                (S_NATIONKEY int) \n");
     fprintf(fp, "                (S_PHONE string) \n");
     fprintf(fp, "                (S_ACCTBAL real) \n");                   
     fprintf(fp, "                (S_COMMENT text) )))\n");
     fprintf(fp, "  ( \n");
   }


   PR_STRT(fp);
   PR_INT(fp, supp->suppkey);
   PR_STR(fp, supp->name, S_NAME_LEN);
   PR_VSTR(fp, supp->address, 
       (columnar)?(long)(ceil(S_ADDR_LEN * V_STR_HGH)):supp->alen);
   PR_INT(fp, supp->nation_code);
   PR_STR(fp, supp->phone, PHONE_LEN);
   PR_MONEY(fp, supp->acctbal);
   PR_TEXT_LAST(fp, supp->comment, 
       (columnar)?(long)(ceil(S_CMNT_LEN * V_STR_HGH)):supp->clen);
   PR_END(fp, GLOB_LAST_RECORD);

   return(0);
}

int
pr_nation(code_t *c, int mode)
{
static FILE *fp = NULL;
        
   if (fp == NULL)
   {
        fp = print_prep(NATION, mode);

     fprintf(fp, "(OBJECT NATION \n");
     fprintf(fp, "  ()");
     fprintf(fp, "  (rel( tuple ( (N_NATIONKEY int) \n");
     fprintf(fp, "                (N_NAME string) \n");
     fprintf(fp, "                (N_REGIONKEY int) \n");                  
     fprintf(fp, "                (N_COMMENT text) )))\n");
     fprintf(fp, "  ( \n");
   }

   PR_STRT(fp);
   PR_INT(fp, c->code);
   PR_STR(fp, c->text, NATION_LEN);
   PR_INT(fp, c->join);
   PR_TEXT_LAST(fp, c->comment, 
       (columnar)?(long)(ceil(N_CMNT_LEN * V_STR_HGH)):c->clen);
   PR_END(fp, GLOB_LAST_RECORD);

   return(0);
}

int
pr_region(code_t *c, int mode)
{
static FILE *fp = NULL;
        
   if (fp == NULL)
   {
        fp = print_prep(REGION, mode);
	
     fprintf(fp, "(OBJECT REGION \n");
     fprintf(fp, "  ()");
     fprintf(fp, "  (rel( tuple ( (R_REGIONKEY int) \n");
     fprintf(fp, "                (R_NAME string) \n");                 
     fprintf(fp, "                (R_COMMENT text) )))\n");
     fprintf(fp, "  ( \n");
   }


   PR_STRT(fp);
   PR_INT(fp, c->code);
   PR_STR(fp, c->text, REGION_LEN);
   PR_TEXT_LAST(fp, c->comment, 
       (columnar)?(long)(ceil(R_CMNT_LEN * V_STR_HGH)):c->clen);
   PR_END(fp, GLOB_LAST_RECORD);

   return(0);
}

/* 
 * NOTE: this routine does NOT use the BCD2_* routines. As a result,
 * it WILL fail if the keys being deleted exceed 32 bits. Since this
 * would require ~660 update iterations, this seems an acceptable
 * oversight
 */
int
pr_drange(int tbl, long min, long cnt, long num)
{
    static int  last_num = 0;
    static FILE *dfp = NULL;
    int child = -1;
    long start, last, new;

	static int rows_per_segment=0;
	static int rows_this_segment=0;
	static int residual_rows=0;

    if (last_num != num)
        {
        if (dfp)
            fclose(dfp);
        dfp = print_prep(tbl, -num);
        if (dfp == NULL)
            return(-1);
        last_num = num;
		rows_this_segment=0;
        }

    start = MK_SPARSE(min, (num - 1)/ (10000 / refresh));
    last = start - 1;
    for (child=min; cnt > 0; child++, cnt--)
        {
        new = MK_SPARSE(child, (num - 1) / (10000 / refresh));
        if (gen_rng == 1 && new - last == 1)
            {
            last = new;
            continue;
            }
	if (gen_sql)
	    {
	    fprintf(dfp, 
		"delete from %s where %s between %ld and %ld;\n",
		    tdefs[ORDER].name, "o_orderkey", start, last);
	    fprintf(dfp, 
		"delete from %s where %s between %ld and %ld;\n",
		    tdefs[LINE].name, "l_orderkey", start, last);
	    fprintf(dfp, "commit work;\n");
	    }
	else 
	    if (gen_rng)
                {
                PR_STRT(dfp);
                PR_INT(dfp, start);
                PR_INT(dfp, last);
                PR_END(dfp, GLOB_LAST_RECORD);
                }
            else
                {
				if (delete_segments)
					{
					if(rows_per_segment==0)
						{
						rows_per_segment = (cnt / delete_segments);
						residual_rows = (cnt % delete_segments);
						rows_per_segment++;
						}
					if(delete_segment <= residual_rows)
						{
						if((++rows_this_segment) > rows_per_segment)
							{
							fclose(dfp);
							dfp = print_prep(tbl, -num);
							if (dfp == NULL) return(-1);
							last_num = num;
							rows_this_segment=1;
							}
						}
					else
						{
						if((++rows_this_segment) >= rows_per_segment)
							{
							fclose(dfp);
							dfp = print_prep(tbl, -num);
							if (dfp == NULL) return(-1);
							last_num = num;
							rows_this_segment=1;
							}
						}
					}
                PR_STRT(dfp);
                PR_KEY(dfp, new);
                PR_END(dfp, GLOB_LAST_RECORD);
                }
	start = new;
	last = new;
        }
    if (gen_rng)
	{
	PR_STRT(dfp);
	PR_INT(dfp, start);
	PR_INT(dfp, last);
	PR_END(dfp, GLOB_LAST_RECORD);
	}
    
    return(0);
}

/*
 * verify functions: routines which replace the pr_routines and generate a pseudo checksum 
 * instead of generating the actual contents of the tables. Meant to allow large scale data 
 * validation without requiring a large amount of storage
 */
int
vrf_cust(customer_t *c, int mode)
{
   VRF_STRT(CUST);
   VRF_INT(CUST, c->custkey);
   VRF_STR(CUST, c->name);
   VRF_STR(CUST, c->address);
   VRF_INT(CUST, c->nation_code);
   VRF_STR(CUST, c->phone);
   VRF_MONEY(CUST, c->acctbal);
   VRF_STR(CUST, c->mktsegment);
   VRF_STR(CUST, c->comment);
   VRF_END(CUST);

   return(0);
}

/*
 * print the numbered order 
 */
int
vrf_order(order_t *o, int mode)
{
    VRF_STRT(ORDER);
    VRF_HUGE(ORDER, o->okey);
    VRF_INT(ORDER, o->custkey);
    VRF_CHR(ORDER, o->orderstatus);
    VRF_MONEY(ORDER, o->totalprice);
    VRF_STR(ORDER, o->odate);
    VRF_STR(ORDER, o->opriority);
    VRF_STR(ORDER, o->clerk);
    VRF_INT(ORDER, o->spriority);
    VRF_STR(ORDER, o->comment);
    VRF_END(ORDER);

    return(0);
}

/*
 * print an order's lineitems
 */
int
vrf_line(order_t *o, int mode)
{
	int i;

    for (i = 0; i < o->lines; i++)
        {
        VRF_STRT(LINE);
        VRF_HUGE(LINE, o->l[i].okey);
        VRF_INT(LINE, o->l[i].partkey);
        VRF_INT(LINE, o->l[i].suppkey);
        VRF_INT(LINE, o->l[i].lcnt);
        VRF_INT(LINE, o->l[i].quantity);
        VRF_MONEY(LINE, o->l[i].eprice);
        VRF_MONEY(LINE, o->l[i].discount);
        VRF_MONEY(LINE, o->l[i].tax);
        VRF_CHR(LINE, o->l[i].rflag[0]);
        VRF_CHR(LINE, o->l[i].lstatus[0]);
        VRF_STR(LINE, o->l[i].sdate);
        VRF_STR(LINE, o->l[i].cdate);
        VRF_STR(LINE, o->l[i].rdate);
        VRF_STR(LINE, o->l[i].shipinstruct);
        VRF_STR(LINE, o->l[i].shipmode);
        VRF_STR(LINE, o->l[i].comment);
        VRF_END(LINE);
        }

   return(0);
}

/*
 * print the numbered order *and* its associated lineitems
 */
int
vrf_order_line(order_t *o, int mode)
{
    vrf_order(o, mode);
    vrf_line(o, mode);

    return(0);
}

/*
 * print the given part
 */
int
vrf_part(part_t *part, int mode)
{

   VRF_STRT(PART);
   VRF_INT(PART, part->partkey);
   VRF_STR(PART, part->name);
   VRF_STR(PART, part->mfgr);
   VRF_STR(PART, part->brand);
   VRF_STR(PART, part->type);
   VRF_INT(PART, part->size);
   VRF_STR(PART, part->container);
   VRF_MONEY(PART, part->retailprice);
   VRF_STR(PART, part->comment);
   VRF_END(PART);

   return(0);
}

/*
 * print the given part's suppliers
 */
int
vrf_psupp(part_t *part, int mode)
{
    long      i;

   for (i = 0; i < SUPP_PER_PART; i++)
      {
      VRF_STRT(PSUPP);
      VRF_INT(PSUPP, part->s[i].partkey);
      VRF_INT(PSUPP, part->s[i].suppkey);
      VRF_INT(PSUPP, part->s[i].qty);
      VRF_MONEY(PSUPP, part->s[i].scost);
      VRF_STR(PSUPP, part->s[i].comment);
      VRF_END(PSUPP);
      }

   return(0);
}

/*
 * print the given part *and* its suppliers
 */
int
vrf_part_psupp(part_t *part, int mode)
{
    vrf_part(part, mode);
    vrf_psupp(part, mode);

    return(0);
}

int
vrf_supp(supplier_t *supp, int mode)
{
   VRF_STRT(SUPP);
   VRF_INT(SUPP, supp->suppkey);
   VRF_STR(SUPP, supp->name);
   VRF_STR(SUPP, supp->address);
   VRF_INT(SUPP, supp->nation_code);
   VRF_STR(SUPP, supp->phone);
   VRF_MONEY(SUPP, supp->acctbal);
   VRF_STR(SUPP, supp->comment); 
   VRF_END(SUPP);

   return(0);
}

int
vrf_nation(code_t *c, int mode)
{
   VRF_STRT(NATION);
   VRF_INT(NATION, c->code);
   VRF_STR(NATION, c->text);
   VRF_INT(NATION, c->join);
   VRF_STR(NATION, c->comment);
   VRF_END(NATION);

   return(0);
}

int
vrf_region(code_t *c, int mode)
{
   VRF_STRT(REGION);
   VRF_INT(REGION, c->code);
   VRF_STR(REGION, c->text);
   VRF_STR(REGION, c->comment);
   VRF_END(fp);

   return(0);
}

