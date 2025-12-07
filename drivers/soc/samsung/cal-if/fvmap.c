#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/io.h>
#include <linux/debugfs.h>
#include <linux/uaccess.h>
#include <linux/kobject.h>
#include <soc/samsung/cal-if.h>
#include <linux/sysfs.h>

#include "fvmap.h"
#include "cmucal.h"
#include "vclk.h"
#include "ra.h"
#include <linux/string.h>

#define FVMAP_SIZE		(SZ_8K)
#define STEP_UV			(6250)

#define MAX_FV_LEVELS   50 // Arbitrary safe limit for table size loop
static struct kobject *fvmap_kobj;

void __iomem *fvmap_base;
void __iomem *sram_fvmap_base;

static int init_margin_table[MAX_MARGIN_ID];
static int volt_offset_percent = 0;
static int percent_margin_table[MAX_MARGIN_ID];

static int __init get_mif_volt(char *str)
{
	int volt;

	get_option(&str, &volt);
	init_margin_table[MARGIN_MIF] = volt;

	return 0;
}
early_param("mif", get_mif_volt);

static int __init get_int_volt(char *str)
{
	int volt;

	get_option(&str, &volt);
	init_margin_table[MARGIN_INT] = volt;

	return 0;
}
early_param("int", get_int_volt);

static int __init get_big_volt(char *str)
{
	int volt;

	get_option(&str, &volt);
	init_margin_table[MARGIN_BIG] = volt;

	return 0;
}
early_param("big", get_big_volt);

static int __init get_mid_volt(char *str)
{
	int volt;

	get_option(&str, &volt);
	init_margin_table[MARGIN_MID] = volt;

	return 0;
}
early_param("mid", get_mid_volt);

static int __init get_lit_volt(char *str)
{
	int volt;

	get_option(&str, &volt);
	init_margin_table[MARGIN_LIT] = volt;

	return 0;
}
early_param("lit", get_lit_volt);

static int __init get_g3d_volt(char *str)
{
	int volt;

	get_option(&str, &volt);
	init_margin_table[MARGIN_G3D] = volt;

	return 0;
}
early_param("g3d", get_g3d_volt);

static int __init get_intcam_volt(char *str)
{
	int volt;

	get_option(&str, &volt);
	init_margin_table[MARGIN_INTCAM] = volt;

	return 0;
}
early_param("intcam", get_intcam_volt);

static int __init get_cam_volt(char *str)
{
	int volt;

	get_option(&str, &volt);
	init_margin_table[MARGIN_CAM] = volt;

	return 0;
}
early_param("cam", get_cam_volt);

static int __init get_disp_volt(char *str)
{
	int volt;

	get_option(&str, &volt);
	init_margin_table[MARGIN_DISP] = volt;

	return 0;
}
early_param("disp", get_disp_volt);

static int __init get_g3dm_volt(char *str)
{
	int volt;

	get_option(&str, &volt);
	init_margin_table[MARGIN_G3DM] = volt;

	return 0;
}
early_param("g3dm", get_g3dm_volt);

static int __init get_cp_volt(char *str)
{
	int volt;

	get_option(&str, &volt);
	init_margin_table[MARGIN_CP] = volt;

	return 0;
}
early_param("cp", get_cp_volt);

static int __init get_fsys0_volt(char *str)
{
	int volt;

	get_option(&str, &volt);
	init_margin_table[MARGIN_FSYS0] = volt;

	return 0;
}
early_param("fsys0", get_fsys0_volt);

static int __init get_aud_volt(char *str)
{
	int volt;

	get_option(&str, &volt);
	init_margin_table[MARGIN_AUD] = volt;

	return 0;
}
early_param("aud", get_aud_volt);

static int __init get_iva_volt(char *str)
{
	int volt;

	get_option(&str, &volt);
	init_margin_table[MARGIN_IVA] = volt;

	return 0;
}
early_param("iva", get_iva_volt);

static int __init get_score_volt(char *str)
{
	int volt;

	get_option(&str, &volt);
	init_margin_table[MARGIN_SCORE] = volt;

	return 0;
}
early_param("score", get_score_volt);

static int __init get_npu_volt(char *str)
{
	int volt;

	get_option(&str, &volt);
	init_margin_table[MARGIN_NPU] = volt;

	return 0;
}
early_param("npu", get_npu_volt);

static int __init get_mfc_volt(char *str)
{
	int volt;

	get_option(&str, &volt);
	init_margin_table[MARGIN_MFC] = volt;

	return 0;
}
early_param("mfc", get_mfc_volt);

static int __init get_dsp_volt(char *str)
{
	int volt;

	get_option(&str, &volt);
	init_margin_table[MARGIN_DSP] = volt;

	return 0;
}
early_param("dsp", get_dsp_volt);

static int __init get_dnc_volt(char *str)
{
	int volt;

	get_option(&str, &volt);
	init_margin_table[MARGIN_DNC] = volt;

	return 0;
}
early_param("dnc", get_dnc_volt);

static int __init get_tnr_volt(char *str)
{
	int volt;

	get_option(&str, &volt);
	init_margin_table[MARGIN_TNR] = volt;

	return 0;
}
early_param("tnr", get_tnr_volt);

static int __init get_percent_margin_volt(char *str)
{
	int percent;

	get_option(&str, &percent);
	volt_offset_percent = percent;

	return 0;
}
early_param("volt_offset_percent", get_percent_margin_volt);

int fvmap_set_raw_voltage_table(unsigned int id, int uV)
{
	struct fvmap_header *fvmap_header;
	struct rate_volt_header *fv_table;
	int num_of_lv;
	int idx, i;

	idx = GET_IDX(id);

	fvmap_header = sram_fvmap_base;
	fv_table = sram_fvmap_base + fvmap_header[idx].o_ratevolt;
	num_of_lv = fvmap_header[idx].num_of_lv;

	for (i = 0; i < num_of_lv; i++)
		fv_table->table[i].volt += uV;

	return 0;
}

int fvmap_get_voltage_table(unsigned int id, unsigned int *table)
{
	struct fvmap_header *fvmap_header = fvmap_base;
	struct rate_volt_header *fv_table;
	int idx, i;
	int num_of_lv;

	if (!IS_ACPM_VCLK(id))
		return 0;

	idx = GET_IDX(id);

	fvmap_header = fvmap_base;
	fv_table = fvmap_base + fvmap_header[idx].o_ratevolt;
	num_of_lv = fvmap_header[idx].num_of_lv;

	for (i = 0; i < num_of_lv; i++)
		table[i] = fv_table->table[i].volt;

	return num_of_lv;

}

int fvmap_get_raw_voltage_table(unsigned int id)
{
	struct fvmap_header *fvmap_header;
	struct rate_volt_header *fv_table;
	int idx, i;
	int num_of_lv;
	unsigned int table[20];

	idx = GET_IDX(id);

	fvmap_header = sram_fvmap_base;
	fv_table = sram_fvmap_base + fvmap_header[idx].o_ratevolt;
	num_of_lv = fvmap_header[idx].num_of_lv;

	for (i = 0; i < num_of_lv; i++)
		table[i] = fv_table->table[i].volt;

	for (i = 0; i < num_of_lv; i++)
		printk("dvfs id : %d  %d Khz : %d uv\n", ACPM_VCLK_TYPE | id, fv_table->table[i].rate, table[i]);

	return 0;
}

static void check_percent_margin(struct rate_volt_header *head, unsigned int num_of_lv)
{
	int org_volt;
	int percent_volt;
	int i;

	if (!volt_offset_percent)
		return;

	for (i = 0; i < num_of_lv; i++) {
		org_volt = head->table[i].volt;
		percent_volt = org_volt * volt_offset_percent / 100;
		head->table[i].volt = org_volt + rounddown(percent_volt, STEP_UV);
	}
}

static int get_vclk_id_from_margin_id(int margin_id)
{
	int size = cmucal_get_list_size(ACPM_VCLK_TYPE);
	int i;
	struct vclk *vclk;

	for (i = 0; i < size; i++) {
		vclk = cmucal_get_node(ACPM_VCLK_TYPE | i);

		if (vclk->margin_id == margin_id)
			return i;
	}

	return -EINVAL;
}

#define attr_percent(margin_id, type)								\
static ssize_t show_##type##_percent								\
(struct kobject *kobj, struct kobj_attribute *attr, char *buf)					\
{												\
	return snprintf(buf, PAGE_SIZE, "%d\n", percent_margin_table[margin_id]);		\
}												\
												\
static ssize_t store_##type##_percent								\
(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count)		\
{												\
	int input, vclk_id;									\
												\
	if (!sscanf(buf, "%d", &input))								\
		return -EINVAL;									\
												\
	if (input < -100 || input > 100)							\
		return -EINVAL;									\
												\
	vclk_id = get_vclk_id_from_margin_id(margin_id);					\
	if (vclk_id == -EINVAL)									\
		return vclk_id;									\
	percent_margin_table[margin_id] = input;						\
	cal_dfs_set_volt_margin(vclk_id | ACPM_VCLK_TYPE, input);				\
												\
	return count;										\
}												\
												\
static struct kobj_attribute type##_percent =							\
__ATTR(type##_percent, 0600,									\
	show_##type##_percent, store_##type##_percent)

attr_percent(MARGIN_MIF, mif_margin);
attr_percent(MARGIN_INT, int_margin);
attr_percent(MARGIN_BIG, big_margin);
attr_percent(MARGIN_MID, mid_margin);
attr_percent(MARGIN_LIT, lit_margin);
attr_percent(MARGIN_G3D, g3d_margin);
attr_percent(MARGIN_INTCAM, intcam_margin);
attr_percent(MARGIN_CAM, cam_margin);
attr_percent(MARGIN_DISP, disp_margin);
attr_percent(MARGIN_CP, cp_margin);
attr_percent(MARGIN_FSYS0, fsys0_margin);
attr_percent(MARGIN_AUD, aud_margin);
attr_percent(MARGIN_IVA, iva_margin);
attr_percent(MARGIN_SCORE, score_margin);
attr_percent(MARGIN_NPU, npu_margin);
attr_percent(MARGIN_MFC, mfc_margin);

static struct attribute *percent_margin_attrs[] = {
	&mif_margin_percent.attr,
	&int_margin_percent.attr,
	&big_margin_percent.attr,
	&mid_margin_percent.attr,
	&lit_margin_percent.attr,
	&g3d_margin_percent.attr,
	&intcam_margin_percent.attr,
	&cam_margin_percent.attr,
	&disp_margin_percent.attr,
	&cp_margin_percent.attr,
	&fsys0_margin_percent.attr,
	&aud_margin_percent.attr,
	&iva_margin_percent.attr,
	&score_margin_percent.attr,
	&npu_margin_percent.attr,
	&mfc_margin_percent.attr,
	NULL,
};

static const struct attribute_group percent_margin_group = {
	.attrs = percent_margin_attrs,
};

#ifdef CONFIG_SEC_FACTORY
#ifdef CONFIG_SOC_EXYNOS9830
#include <soc/samsung/ect_parser.h>
#include <soc/samsung/cal-if.h>
#include "asv.h"

#define VUP_0 12500
#define VUP_1 25000

enum spec_volt_type {
	ASV_G_FUSED = 0,
	ASV_G_GRP
};

static int asv_g_yield_volt[4];

static int __init get_mvup(char *str)
{
	asv_g_yield_volt[1] = VUP_0;

	return 0;
}
early_param("mvup", get_mvup);

static int __init get_bvup(char *str)
{
	asv_g_yield_volt[2] = VUP_1;

	return 0;
}
early_param("bvup", get_bvup);

static int __init get_gvup(char *str)
{
	asv_g_yield_volt[3] = VUP_1;

	return 0;
}
early_param("gvup", get_gvup);

static ssize_t show_asv_g_spec(int id, enum spec_volt_type type, char *buf)
{
	void *gen_block;
	struct ect_gen_param_table *spec;
	int asv_tbl_ver, asv_grp, tbl_size, j, vtyp_freq, num_lv;
	unsigned int fused_volt, grp_volt = 0, volt;
	struct dvfs_rate_volt rate_volt[48];
	unsigned int (*spec_table)[10];
	char *spec_table_name[4] = { "SPEC_CPUCL0", "SPEC_CPUCL1", "SPEC_CPUCL2", "SPEC_G3D" };
	int cal_id[4] = { ACPM_VCLK_TYPE | 2, ACPM_VCLK_TYPE | 3, ACPM_VCLK_TYPE | 4, ACPM_VCLK_TYPE | 10 };
	ssize_t size = 0;

	asv_tbl_ver = asv_get_table_ver();
	if (asv_tbl_ver < 3) {
		pr_err("%s: Do not support ASV-G under ASV table version 3\n", __func__);
		goto out;
	}

	gen_block = ect_get_block("GEN");
	if (gen_block == NULL) {
		pr_err("%s: Failed to get gen block from ECT\n", __func__);
		goto out;
	}

	asv_grp = asv_get_grp(cal_id[id]);
	if (!asv_grp) {
		pr_err("%s: There has no ASV-G information for %s group 0\n", __func__, spec_table_name[id]);
		goto out;
	}

	spec = ect_gen_param_get_table(gen_block, spec_table_name[id]);
	if (spec == NULL) {
		pr_err("%s: Failed to get spec table from ECT\n", __func__);
		goto out;
	}
	spec_table = (unsigned int (*)[10])spec->parameter;

	for (j = 0; j < spec->num_of_row; j++) {
		if (spec_table[j][0] == asv_tbl_ver) {
			grp_volt = spec_table[j][asv_grp + 1];
			vtyp_freq = spec_table[j][1];
			break;
		}
	}

	if (!grp_volt) {
		pr_err("%s: Failed to get grp volt\n", __func__);
		goto out;
	}

	num_lv = cal_dfs_get_lv_num(cal_id[id]);
	tbl_size = cal_dfs_get_rate_asv_table(cal_id[id], rate_volt);
	if (!tbl_size) {
		pr_err("%s: Failed to get asv table\n", __func__);
		goto out;
	}

	for (j = 0; j < num_lv; j++){
		if (rate_volt[j].rate == vtyp_freq) {
			fused_volt = rate_volt[j].volt;
			break;
		}
	}
	if (j == num_lv) {
		pr_err("%s: There has no frequency %d on %d domain\n", __func__, vtyp_freq, spec_table_name[id]);
		goto out;
	}

	fused_volt -= asv_g_yield_volt[id];

	volt = (type == ASV_G_FUSED) ? fused_volt : grp_volt;

	size += snprintf(buf + size, PAGE_SIZE, "%d\n", volt);
out:

	return size;
}

#define asv_g_spec(domain, id)									\
static ssize_t show_asv_g_spec_##domain##_fused_volt						\
(struct kobject *kobj, struct kobj_attribute *attr, char *buf)					\
{												\
	return show_asv_g_spec(id, ASV_G_FUSED, buf);						\
}												\
static struct kobj_attribute asv_g_spec_##domain##_fused_volt =					\
__ATTR(domain##_fused_volt, 0400, show_asv_g_spec_##domain##_fused_volt, NULL);	\
static ssize_t show_asv_g_spec_##domain##_grp_volt						\
(struct kobject *kobj, struct kobj_attribute *attr, char *buf)					\
{												\
	return show_asv_g_spec(id, ASV_G_GRP, buf);						\
}												\
static struct kobj_attribute asv_g_spec_##domain##_grp_volt =					\
__ATTR(domain##_grp_volt, 0400, show_asv_g_spec_##domain##_grp_volt, NULL)

#define asv_g_spec_attr(domain)									\
	&asv_g_spec_##domain##_fused_volt.attr,							\
	&asv_g_spec_##domain##_grp_volt.attr

asv_g_spec(cpucl0, 0);
asv_g_spec(cpucl1, 1);
asv_g_spec(cpucl2, 2);
asv_g_spec(g3d, 3);

static struct attribute *asv_g_spec_attrs[] = {
	asv_g_spec_attr(cpucl0),
	asv_g_spec_attr(cpucl1),
	asv_g_spec_attr(cpucl2),
	asv_g_spec_attr(g3d),
	NULL,
};

static const struct attribute_group asv_g_spec_grp = {
	.attrs = asv_g_spec_attrs,
};
#endif
#endif /* CONFIG_SEC_FACTORY */

#ifdef CONFIG_SOC_EXYNOS9830_UNDERVOLT
// Define domain IDs for undervolting
#define EXYNOS9830_DOMAIN_ID_CPUCL0 3 	// Set domain_id for CPUCL0 here.
#define EXYNOS9830_DOMAIN_ID_CPUCL1 16 	// Set domain_id for CPUCL1 here.
#define EXYNOS9830_DOMAIN_ID_CPUCL2 2 	// Set domain_id for CPUCL2 here.
#define EXYNOS9830_DOMAIN_ID_G3D 4 	// Set domain_id for GPU (G3D)
#define EXYNOS9830_DOMAIN_ID_INTG3D 8 	// Set domain_id for GPU (INTG3D)
#endif

static void fvmap_copy_from_sram(void __iomem *map_base, void __iomem *sram_base)
{
	volatile struct fvmap_header *fvmap_header, *header;
	struct rate_volt_header *old, *new;
	struct clocks *clks;
	struct pll_header *plls;
	struct vclk *vclk;
	unsigned int member_addr;
	unsigned int blk_idx;
	int size, margin;
	int i, j;

	fvmap_header = map_base;
	header = sram_base;

	size = cmucal_get_list_size(ACPM_VCLK_TYPE);

	for (i = 0; i < size; i++) {
		/* load fvmap info */
		fvmap_header[i].dvfs_type = header[i].dvfs_type;
		fvmap_header[i].num_of_lv = header[i].num_of_lv;
		fvmap_header[i].num_of_members = header[i].num_of_members;
		fvmap_header[i].num_of_pll = header[i].num_of_pll;
		fvmap_header[i].num_of_mux = header[i].num_of_mux;
		fvmap_header[i].num_of_div = header[i].num_of_div;
		fvmap_header[i].gearratio = header[i].gearratio;
		fvmap_header[i].init_lv = header[i].init_lv;
		fvmap_header[i].num_of_gate = header[i].num_of_gate;
		fvmap_header[i].reserved[0] = header[i].reserved[0];
		fvmap_header[i].reserved[1] = header[i].reserved[1];
		fvmap_header[i].block_addr[0] = header[i].block_addr[0];
		fvmap_header[i].block_addr[1] = header[i].block_addr[1];
		fvmap_header[i].block_addr[2] = header[i].block_addr[2];
		fvmap_header[i].o_members = header[i].o_members;
		fvmap_header[i].o_ratevolt = header[i].o_ratevolt;
		fvmap_header[i].o_tables = header[i].o_tables;

		vclk = cmucal_get_node(ACPM_VCLK_TYPE | i);
		if (vclk == NULL)
			continue;
		pr_info("dvfs_type : %s - id : %x\n",
				vclk->name, fvmap_header[i].dvfs_type);
		pr_info("  num_of_lv      : %d\n", fvmap_header[i].num_of_lv);
		pr_info("  num_of_members : %d\n", fvmap_header[i].num_of_members);

		old = sram_base + fvmap_header[i].o_ratevolt;
		new = map_base + fvmap_header[i].o_ratevolt;

		check_percent_margin(old, fvmap_header[i].num_of_lv);

		margin = init_margin_table[vclk->margin_id];
		if (margin)
			cal_dfs_set_volt_margin(i | ACPM_VCLK_TYPE, margin);

		for (j = 0; j < fvmap_header[i].num_of_members; j++) {
			clks = sram_base + fvmap_header[i].o_members;

			if (j < fvmap_header[i].num_of_pll) {
				plls = sram_base + clks->addr[j];
				member_addr = plls->addr - 0x90000000;
			} else {

				member_addr = (clks->addr[j] & ~0x3) & 0xffff;
				blk_idx = clks->addr[j] & 0x3;

				member_addr |= ((fvmap_header[i].block_addr[blk_idx]) << 16) - 0x90000000;
			}


			vclk->list[j] = cmucal_get_id_by_addr(member_addr);

			if (vclk->list[j] == INVALID_CLK_ID)
				pr_info("  Invalid addr :0x%x\n", member_addr);
			else
				pr_info("  DVFS CMU addr:0x%x\n", member_addr);
		}

		for (j = 0; j < fvmap_header[i].num_of_lv; j++) {
			new->table[j].rate = old->table[j].rate;
			new->table[j].volt = old->table[j].volt;
			pr_info("  lv : [%7d], volt = %d uV (%d %%) \n",
					new->table[j].rate, new->table[j].volt,
					volt_offset_percent);
		}

#ifdef CONFIG_SOC_EXYNOS9830_UNDERVOLT
/* Correção: Movido o bloco de undervolt para DENTRO do loop 'for (i = 0; i < size; i++)'
   para que 'vclk' e 'old' estejam no escopo correto. */
#if CONFIG_SOC_EXYNOS9830_CL0_UV != 0
				/* Apply undervolt if the domain is CPUCL0 */
				if (vclk && strcmp(vclk->name, "CPUCL0") == 0) {
					for (j = 0; j < fvmap_header[i].num_of_lv; j++) {
						old->table[j].volt = (old->table[j].volt * (100 - CONFIG_SOC_EXYNOS9830_CL0_UV)) / 100;
					}
				}
#endif

#if CONFIG_SOC_EXYNOS9830_CL1_UV != 0
				/* Apply undervolt if the domain is CPUCL1 */
				if (vclk && strcmp(vclk->name, "CPUCL1") == 0) {
					for (j = 0; j < fvmap_header[i].num_of_lv; j++) {
				if (old->table[j].rate == 2704000 || old->table[j].rate == 2600000)
#if CONFIG_SOC_EXYNOS9830_CL1_UV_OC != 0
					old->table[j].volt = (old->table[j].volt * (100 - CONFIG_SOC_EXYNOS9830_CL1_UV_OC)) / 100;
#else
					old->table[j].volt = (old->table[j].volt * (100 - CONFIG_SOC_EXYNOS9830_CL1_UV)) / 100;
#endif
				else
					old->table[j].volt = (old->table[j].volt * (100 - CONFIG_SOC_EXYNOS9830_CL1_UV)) / 100;
					}
				}
#endif

#if CONFIG_SOC_EXYNOS9830_CL2_UV != 0
				/* Apply undervolt if the domain is CPUCL2 */
				if (vclk && strcmp(vclk->name, "CPUCL2") == 0) {
					for (j = 0; j < fvmap_header[i].num_of_lv; j++) {
						old->table[j].volt = (old->table[j].volt * (100 - CONFIG_SOC_EXYNOS9830_CL2_UV)) / 100;
					}
				}
#endif

#if CONFIG_SOC_EXYNOS9830_GPU_UV != 0
				/* Apply undervolt if the domain is G3D or INTG3D */
				if (vclk && (strcmp(vclk->name, "G3D") == 0 || strcmp(vclk->name, "intg3d") == 0)) {
					for (j = 0; j < fvmap_header[i].num_of_lv; j++) {
						old->table[j].volt = (old->table[j].volt * (100 - CONFIG_SOC_EXYNOS9830_GPU_UV)) / 100;
					}
				}
#endif
#endif
	} // Fim do loop for (i = 0; i < size; i++)
} // Fim da função fvmap_copy_from_sram

        /* Print the frequency-voltage table for this domain */
        pr_info("Frequency-Voltage Table for Domain %s (ID: %d):\n", vclk->name, fvmap_header[i].domain_id);
        for (j = 0; j < fvmap_header[i].num_of_lv; j++) {
            pr_info("  Level %d: %d kHz, %d uV\n", j, old->table[j].rate, old->table[j].volt * STEP_UV);
        }

static ssize_t show_fv_table(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
	struct fvmap_header *fvmap_header;
	struct rate_volt_header *fv_table;
	int idx = -1, i, num_of_lv, margin_id = -1, vclk_id;
	ssize_t len = 0;
	const char *name = attr->attr.name;
	void __iomem *base_to_read = fvmap_base;

	if (!base_to_read) {
		pr_err("fvmap: base not initialized\n");
		return -EIO;
	}

	// Determine margin_id based on attribute name
	if (strcmp(name, "cpucl0_fv_table") == 0) {
		margin_id = MARGIN_CPUCL0;
	} else if (strcmp(name, "cpucl1_fv_table") == 0) {
		margin_id = MARGIN_CPUCL1;
	} else if (strcmp(name, "cpucl2_fv_table") == 0) {
		margin_id = MARGIN_CPUCL2;
	} else if (strcmp(name, "g3d_fv_table") == 0) {
		margin_id = MARGIN_INTG3D;
	} else {
		pr_err("fvmap: Unknown attribute %s\n", name);
		return -EINVAL;
	}

	vclk_id = get_vclk_id_from_margin_id(margin_id);
	if (vclk_id < 0) {
		pr_err("fvmap: Could not find vclk_id for margin_id %d (name: %s)\n", margin_id, name);
		return -EINVAL;
	}
	idx = GET_IDX(vclk_id | ACPM_VCLK_TYPE);


	fvmap_header = (struct fvmap_header *)base_to_read; // Cast needed

	if (idx < 0 || idx >= cmucal_get_list_size(ACPM_VCLK_TYPE)) {
		 pr_err("fvmap: Invalid index %d derived for margin_id %d\n", idx, margin_id);
		 return -EINVAL;
	}

	fv_table = (struct rate_volt_header *)(base_to_read + fvmap_header[idx].o_ratevolt);
	num_of_lv = fvmap_header[idx].num_of_lv;

	if (num_of_lv <= 0 || num_of_lv > MAX_FV_LEVELS) {
		 pr_err("fvmap: Invalid num_of_lv %d for index %d\n", num_of_lv, idx);
		 return -EINVAL;
	}


	len += scnprintf(buf + len, PAGE_SIZE - len, "Freq(kHz)\tVolt(uV)\n");

	for (i = 0; i < num_of_lv; i++) {
		if (len >= PAGE_SIZE - 30) {
			pr_warn_ratelimited("fvmap: Buffer full when printing FV table for %s\n", name);
			break;
		}
		// Read rate and volt (in STEP_UV units) and convert volt to uV
		len += scnprintf(buf + len, PAGE_SIZE - len, "%-10u\t%-10u\n",
						 fv_table->table[i].rate,
						 fv_table->table[i].volt * STEP_UV);
	}

	return len;
}

static struct kobj_attribute cpucl0_fv_table_attr =
	__ATTR(cpucl0_fv_table, 0444, show_fv_table, NULL);
static struct kobj_attribute cpucl1_fv_table_attr =
	__ATTR(cpucl1_fv_table, 0444, show_fv_table, NULL);
static struct kobj_attribute cpucl2_fv_table_attr =
	__ATTR(cpucl2_fv_table, 0444, show_fv_table, NULL);
static struct kobj_attribute intg3d_fv_table_attr =
	__ATTR(g3d_fv_table, 0444, show_fv_table, NULL);

static struct attribute *fvmap_attrs[] = {
	&cpucl0_fv_table_attr.attr,
	&cpucl1_fv_table_attr.attr,
	&cpucl2_fv_table_attr.attr,
	&g3d_fv_table_attr.attr,
	NULL,
};

static const struct attribute_group fvmap_group = {
	.attrs = fvmap_attrs,
	.name = "fv_tables",
};

int fvmap_init(void __iomem *sram_base)
{
	void __iomem *map_base;
	struct kobject *kobj;
#ifdef CONFIG_SEC_FACTORY
	struct kobject *asv_g_kobj;
#endif /* CONFIG_SEC_FACTORY */

	map_base = kzalloc(FVMAP_SIZE, GFP_KERNEL);
	if (!map_base) {
		pr_err("%s: Failed to allocate memory for fvmap\n", __func__);
		return -ENOMEM;
	}

	fvmap_base = map_base;
	sram_fvmap_base = sram_base;
	pr_info("%s:fvmap initialize %p\n", __func__, sram_base);
	fvmap_copy_from_sram(map_base, sram_base);

	/* percent margin for each doamin at runtime */
	kobj = kobject_create_and_add("percent_margin", power_kobj);
	if (!kobj) {
		pr_err("Fail to create percent_margin kboject\n");
	} else {
		if (sysfs_create_group(kobj, &percent_margin_group))
			pr_err("Fail to create percent_margin group\n");
	}

	kobj = kobject_create_and_add("asv-g", kernel_kobj);
	if (!kobj) {
		pr_err("Fail to create asv-g kboject\n");
	} else {
		if (sysfs_create_group(kobj, &asv_g_spec_grp))
			pr_err("Fail to create asv_g_spec group\n");
	}

	fvmap_kobj = kobject_create_and_add("fvmap", kernel_kobj);
	if (!fvmap_kobj) {
		pr_err("fvmap: Failed to create fvmap kobject\n");
	} else {
		if (sysfs_create_group(fvmap_kobj, &fvmap_group)) {
			pr_err("fvmap: Failed to create fvmap sysfs group\n");
		}
	}

	return 0;
}
/* Correção: Removida a chave '}' extra que estava aqui. */
