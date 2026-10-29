/**
 * Manual function overrides and ICALL diagnostics
 *
 * This file provides:
 *   - recomp_lookup_manual()  : intercept specific Xbox VAs with hand-written code
 *   - recomp_icall_fail_log() : log when an indirect call target can't be resolved
 *   - ICALL trace ring buffer  : globals used by the RECOMP_ICALL macro
 *
 * The recomp pipeline generates an auto-dispatch table (recomp_lookup) that
 * resolves most function addresses. recomp_lookup_manual() is called FIRST,
 * giving you a chance to override any function with a custom implementation.
 *
 * Common reasons to add manual overrides:
 *   - Trace a function to understand call flow (wrap the generated version)
 *   - Fix a function the lifter translated incorrectly
 *   - Stub out a function that crashes (return early, set eax to a safe value)
 *   - Redirect a function to a native implementation (e.g., skip CRT init)
 *   - Intercept D3D/audio calls for custom rendering or sound
 */

#include <stdio.h>
#include "recomp_env.h"
#include <stdint.h>
#include <stddef.h>   /* ptrdiff_t */
#include "nv2a_backend_common.h"  /* nv2a_host_opts: display.aspect, the sub_0009AD80 wrapper */

/* ── ICALL trace ring buffer ───────────────────────────────── */

/*
 * These globals are written by the RECOMP_ICALL macro (defined in
 * recomp_types.h) every time an indirect call is dispatched. When a
 * crash occurs, the VEH handler or recomp_icall_fail_log() can dump
 * the last 16 call targets to help you trace what happened.
 *
 * The runtime owns them: xbox_kernel defines all three in
 * src/kernel/xbox_memory_layout.c, and recomp_types.h declares them extern.
 * Declare, do not define -- a definition here as well is a duplicate symbol,
 * and a project copied from this template failed to link on all three:
 *
 *   xbox_memory_layout.obj : error LNK2005: g_icall_count already defined
 *                            in recomp_manual.obj
 */
extern volatile uint32_t g_icall_trace[16];
extern volatile uint32_t g_icall_trace_idx;
extern volatile uint64_t g_icall_count;

typedef void (*recomp_func_t)(void);

/* ── Register state (defined in xbox_memory_layout.c) ──────── */

/* The generated header, for the register model, MEM32 and RECOMP_ABI_CALL.
 * The registers are thread-local there, so a plain `extern uint32_t g_eax`
 * here would not even name the same object. */
#include "gen/recomp_types.h"

/* ── Manual function overrides ─────────────────────────────── */

/*
 * Return a function pointer to override the given Xbox VA, or NULL
 * to fall through to the auto-generated dispatch table.
 *
 * This is called on every indirect call (RECOMP_ICALL) and every
 * direct call through the dispatch table, so keep it fast. A chain
 * of if-statements on uint32_t compiles to a simple comparison
 * sequence; for large override tables, consider a sorted array
 * with binary search.
 *
 * Examples of common override patterns:
 *
 *   // Trace wrapper: log entry/exit around the generated function
 *   extern void sub_00012345(void);
 *   static void traced_sub_00012345(void) {
 *       fprintf(stderr, "[TRACE] sub_00012345 entered, eax=0x%08X\n", g_eax);
 *       sub_00012345();
 *       fprintf(stderr, "[TRACE] sub_00012345 returned, eax=0x%08X\n", g_eax);
 *   }
 *
 *   // Stub: skip a function entirely (return 0 in eax)
 *   static void stub_00067890(void) {
 *       g_eax = 0;
 *   }
 *
 *   // Fix: replace a broken lifted function with correct C
 *   static void fixed_sub_000ABCDE(void) {
 *       // Read arguments from stack/registers per calling convention
 *       uint32_t arg1 = g_ecx;
 *       uint32_t arg2 = MEM32(g_esp + 4);
 *       // ... correct implementation ...
 *       g_eax = result;
 *   }
 */
recomp_func_t recomp_lookup_manual(uint32_t xbox_va)
{
    /*
     * TODO: Add your overrides here. Examples:
     *
     * if (xbox_va == 0x00012345) return traced_sub_00012345;
     * if (xbox_va == 0x00067890) return stub_00067890;
     * if (xbox_va == 0x000ABCDE) return fixed_sub_000ABCDE;
     */

    (void)xbox_va;
    return (recomp_func_t)0;
}

/* ── Hor+: the visibility test's side planes ─────────────────
 *
 * sub_0009AD80(x, y, z, r) is the game's bounding-sphere test: it moves the
 * centre into view space and projects it with the stored projection's _11
 * (0x3AE6A0) and _22 (0x3AE6B4), then keeps the sphere only while the
 * projected centre lies within its projected radius of the +-1 edges
 * (|x| <= 1 + r, the same for y). The model
 * and node draws (sub_0009CE20 and the others) skip whatever it rejects.
 *
 * Hor+ pads the 4:3 targets out to display.aspect on the host, so the screen
 * shows NDC x out to +-A/(4/3), but this test still stops at +-1: objects past
 * the old 4:3 edge are culled and pop in only when they cross it. Scaling _11
 * by s = (4/3)/A for the call turns the test into |x| <= 1/s + r, the padded
 * width, exactly, and scales the projected radius with it. _22 is untouched:
 * Hor+ keeps the vertical extent.
 *
 * The scaled value lives only for the call. sub_00135D60 copied the matrix
 * into the device when sub_00092470 built it, so the GPU projection never
 * sees it, and the other readers of 0x3AE6A0 (sub_0009BF20/BF3A, which place
 * sprites by _11 * 320) run outside it. It is put back only if nothing has
 * rewritten it since, so a projection rebuilt meanwhile wins.
 * The far cull in the same function (0x3AE708 set: vz - r > the cutoff at
 * 0x1D7394, 400 unless the draw-distance mod scales it) reads no _11, so it
 * is unchanged.
 *
 * The main game thread is the only caller and the only reader of 0x3AE6A0:
 * the builder, this test and the sprite placers are all its render code. So
 * although the gen body has back edges where preemption can hand the guest
 * CPU to another thread, no other thread can see the scaled value. The three
 * guest workers (0xF9820, 0xF9860, 0xF98D0) reach no reader by direct calls;
 * their callback ICALL (target from 0x42410C) is the one path not checked.
 * The same reason lets the lazy init go without a lock. A mod that reads the
 * projection from another thread would break this.
 *
 * 4:3, narrower, or a build without the layer (display.aspect stays 0):
 * s_cull_scale stays 0 and the call is the generated body alone, stock. */
extern void sub_0009AD80_gen(void);

static int   s_cull_init;
static float s_cull_scale;              /* 0: stock */

static void cull_init(void)
{
    const struct nv2a_host_opts *o = nv2a_host_opts();

    /* enhance.c's own wide test: it already refuses 4:3 and narrower */
    if (o && o->aspect_w && o->aspect_h && o->aspect_w * 3u > o->aspect_h * 4u) {
        s_cull_scale = (float)(4.0 * o->aspect_h / (3.0 * o->aspect_w));
        fprintf(stderr, "[HORCULL] display.aspect=%u:%u: visibility side planes x%.4f\n",
                o->aspect_w, o->aspect_h, 1.0 / s_cull_scale);
    }
    s_cull_init = 1;
}

void sub_0009AD80(void)
{
    float p11, scaled;

    if (!s_cull_init)
        cull_init();
    if (!s_cull_scale) {
        RECOMP_ABI_CALL(0x0009AD80u, sub_0009AD80_gen);
        return;
    }
    p11 = MEMF(0x3AE6A0);
    scaled = p11 * s_cull_scale;
    MEMF(0x3AE6A0) = scaled;
    RECOMP_ABI_CALL(0x0009AD80u, sub_0009AD80_gen);
    if (MEMF(0x3AE6A0) == scaled)
        MEMF(0x3AE6A0) = p11;
}

/* ── The mod frame: the pad poll ─────────────────────────────
 *
 * sub_000A2DE0 reads the pads (sub_00076E20, XAPI) once per main-loop
 * iteration, after the vblank wait (sub_00138620) and before the frame's
 * update and draws. The toolkit's mods run from xbox_mod_frame, which must
 * be called once per guest frame on the guest's thread: here, after the
 * poll, so a mod's writes land before the draws that read them. Without the
 * mod API it does nothing. */
#include "draw_distance.h"

extern void sub_000A2DE0_gen(void);

void sub_000A2DE0(void)
{
    RECOMP_ABI_CALL(0x000A2DE0u, sub_000A2DE0_gen);
    draw_distance_frame();
}

/* ── ICALL failure logging ─────────────────────────────────── */

/*
 * Called when RECOMP_ICALL cannot resolve a target address.
 * This usually means one of:
 *   - A vtable dispatch to an address not in the dispatch table
 *   - A function pointer loaded from uninitialized or corrupt memory
 *   - A kernel thunk address that the bridge doesn't handle
 *
 * During early bring-up you will see many of these. Most are harmless
 * (the ICALL macro pops the dummy return address and continues).
 * Focus on the ones that cause crashes or incorrect behavior.
 */
void recomp_icall_fail_log(uint32_t va)
{
    fprintf(stderr, "[ICALL] Failed to resolve VA 0x%08X (total calls: %llu)\n",
            va, (unsigned long long)g_icall_count);

    /* Dump last 16 call targets from the ring buffer */
    fprintf(stderr, "  Recent ICALL targets:\n");
    for (int i = 0; i < 16; i++) {
        int idx = (g_icall_trace_idx - 16 + i) & 15;
        if (g_icall_trace[idx])
            fprintf(stderr, "    [%2d] 0x%08X\n", i, g_icall_trace[idx]);
    }
    fflush(stderr);
}

/* An indirect call whose target is not code: a null or wild function pointer.
 *
 * Skipping these is right -- calling a data address is worse -- but skipping
 * them *silently* is not. They almost always arrive inside a loop, so the
 * symptom is a hang with no output rather than a diagnosable null vtable call.
 *
 * Rate-limited per address: a spin can produce millions of these, and the
 * useful information is which addresses occur, not how often.
 */
void recomp_icall_not_code_log(uint32_t va)
{
    enum { SLOTS = 16 };
    static uint32_t seen[SLOTS];
    static uint64_t hits[SLOTS];
    static int count;
    int i;

    for (i = 0; i < count; i++)
        if (seen[i] == va)
            break;
    if (i == count) {
        if (count == SLOTS)
            return;
        seen[count] = va;
        hits[count] = 0;
        count++;
    }
    hits[i]++;
    /* Report at 1, 10, 100, 1000 ... rather than once. A single line says a
     * wild pointer was skipped; the progression says it is being skipped in a
     * loop, which is the difference between a curiosity and the reason the
     * title is hung. */
    {
        uint64_t n = hits[i];
        while (n >= 10 && n % 10 == 0)
            n /= 10;
        if (n != 1)
            return;
    }
    fprintf(stderr, "[ICALL] target 0x%08X is not code -- skipped %llu time(s) "
                    "(null or wild function pointer, at call #%llu)\n",
            va, (unsigned long long)hits[i],
            (unsigned long long)g_icall_count);
    fflush(stderr);
}

/* ── Untranslated instructions ───────────────────────────────────────────
 *
 * The lifter emits RECOMP_UNIMPL(text, va) at every instruction it has no
 * translation for, in place of the bare comment it used to leave. The
 * instruction is still a no-op; this only stops the omission being silent.
 * RECOMP_UNIMPL_TRAP=1 aborts at the first hit, at the guest address of the
 * cause rather than wherever the damage surfaces. */
#include <stdlib.h>

void recomp_unimpl(const char *text, uint32_t va)
{
    static int printed;
    const char *trap = recomp_env(RENV_UNIMPL_TRAP);
    int stop = trap && *trap && *trap != '0';

    if (printed < 50 || stop) {
        printed++;
        fprintf(stderr,
                "[UNIMPL] untranslated instruction REACHED: `%s` at 0x%08X"
                " (a no-op; set RECOMP_DEBUG=unimpl_trap to stop here)\n",
                text, va);
        fflush(stderr);
    }
    if (stop) abort();
}
