#!/usr/bin/env python3
"""Builds scenes/swordfish.xml from the converted parts (parts.tsv):
textured ship meshes, an emissive-sphere starfield and the laser beam."""
import math
import os
import random

HERE = os.path.dirname(os.path.abspath(__file__))
PREFIX = "scenes/swordfish"  # root-relative location of the assets
OUT = os.path.join(os.path.dirname(HERE), "swordfish.xml")

# --- camera ---
LOOK_FROM = (2.6, 1.7, -3.4)
LOOK_AT = (0.0, -0.1, 0.0)
UP = (0.0, 1.0, 0.0)
FOVY = 50.0
W_RES, H_RES = 800, 600

# n x n samples per pixel (override: SAMPLES=1 for a fast preview).
SAMPLES = int(os.environ.get("SAMPLES", "4"))

# --- stars ---
N_STARS = 550
SEED = 7

# --- thrust laser beam ---
BEAM_LENGTH = 16.0         # long enough to run off the edge of the frame
BEAM_RADIUS = 0.032        # thin, laser-like core
BEAM_START_OFFSET = 0.02   # push the beam start just outside the nozzle
BEAM_SEGMENTS = 20
# Beam color is overridable from the environment so you can test colors without
# editing this file:  BEAM_COLOR="1 0.25 0.2" python3 generate_scene.py
BEAM_EMISSION = os.environ.get("BEAM_COLOR", "1 0.16 0.12")      # core color (v1)
GLOW_COLOR = os.environ.get("GLOW_COLOR", BEAM_EMISSION)        # halo color
BEAM_LIGHT_I = "1 0.06 0.06"
GLOW_HALF_WIDTH = 0.42     # half-width of the camera-facing glow billboard

# Shared emissive star materials: (name, emission, weight).
STAR_MATERIALS = [
    ("star_faint", "0.45 0.45 0.5", 40),
    ("star_soft", "0.75 0.75 0.8", 28),
    ("star_mid", "1.05 1.05 1.05", 16),
    ("star_bright", "1.7 1.7 1.7", 6),
    ("star_blue", "0.6 0.75 1.2", 6),
    ("star_warm", "1.2 0.95 0.7", 4),
]


def esc(s):
    return (s.replace("&", "&amp;").replace("<", "&lt;")
             .replace(">", "&gt;").replace('"', "&quot;"))


def sub(a, b):
    return (a[0] - b[0], a[1] - b[1], a[2] - b[2])


def cross(a, b):
    return (a[1] * b[2] - a[2] * b[1],
            a[2] * b[0] - a[0] * b[2],
            a[0] * b[1] - a[1] * b[0])


def norm(a):
    m = math.sqrt(a[0] * a[0] + a[1] * a[1] + a[2] * a[2]) or 1.0
    return (a[0] / m, a[1] / m, a[2] / m)


def build_beam():
    """Reads the engine anchor and writes a cylinder OBJ for the laser beam.

    Returns (material_lines, object_lines, light_lines) or ([],[],[]) if there
    is no anchor file.
    """
    anchor = os.path.join(HERE, "beam_anchor.txt")
    if not os.path.exists(anchor):
        return [], [], []
    with open(anchor) as f:
        rows = [r.split() for r in f.read().strip().splitlines()]
    nozzle = tuple(float(v) for v in rows[0])
    axis = norm(tuple(float(v) for v in rows[1]))

    # Orthonormal basis around the beam axis.
    seed_vec = (1.0, 0.0, 0.0) if abs(axis[0]) < 0.9 else (0.0, 1.0, 0.0)
    u = norm(cross(axis, seed_vec))
    v = cross(axis, u)

    base = tuple(nozzle[i] + axis[i] * BEAM_START_OFFSET for i in range(3))
    tip = tuple(base[i] + axis[i] * BEAM_LENGTH for i in range(3))

    # --- camera-facing glow billboard (additive halo) ---
    mid = tuple((base[i] + tip[i]) * 0.5 for i in range(3))
    to_cam = norm(sub(LOOK_FROM, mid))
    gw = norm(cross(axis, to_cam))  # width direction, in the view plane
    hw = GLOW_HALF_WIDTH
    corners = [
        (tuple(base[i] - gw[i] * hw for i in range(3)), (0.0, 0.0)),
        (tuple(tip[i] - gw[i] * hw for i in range(3)), (1.0, 0.0)),
        (tuple(tip[i] + gw[i] * hw for i in range(3)), (1.0, 1.0)),
        (tuple(base[i] + gw[i] * hw for i in range(3)), (0.0, 1.0)),
    ]
    glow_obj_path = os.path.join(HERE, "obj", "beam_glow.obj")
    with open(glow_obj_path, "w") as f:
        f.write("# laser glow billboard\n")
        for (p, _uv) in corners:
            f.write(f"v {p[0]:.5f} {p[1]:.5f} {p[2]:.5f}\n")
        for (_p, uvc) in corners:
            f.write(f"vt {uvc[0]:.4f} {uvc[1]:.4f}\n")
        f.write("f 1/1 2/2 3/3\n")
        f.write("f 1/1 3/3 4/4\n")

    verts = []
    for k in range(BEAM_SEGMENTS):
        ang = 2.0 * math.pi * k / BEAM_SEGMENTS
        off = tuple((u[i] * math.cos(ang) + v[i] * math.sin(ang)) * BEAM_RADIUS
                    for i in range(3))
        verts.append(tuple(base[i] + off[i] for i in range(3)))  # base ring
    for k in range(BEAM_SEGMENTS):
        ang = 2.0 * math.pi * k / BEAM_SEGMENTS
        off = tuple((u[i] * math.cos(ang) + v[i] * math.sin(ang)) * BEAM_RADIUS
                    for i in range(3))
        verts.append(tuple(tip[i] + off[i] for i in range(3)))   # top ring
    verts.append(tip)  # tip center (index for cap fan)

    obj_path = os.path.join(HERE, "obj", "beam.obj")
    with open(obj_path, "w") as f:
        f.write("# laser beam\n")
        for vv in verts:
            f.write(f"v {vv[0]:.5f} {vv[1]:.5f} {vv[2]:.5f}\n")
        n = BEAM_SEGMENTS
        for k in range(n):
            b0 = k + 1
            b1 = (k + 1) % n + 1
            t0 = n + k + 1
            t1 = n + (k + 1) % n + 1
            f.write(f"f {b0} {b1} {t1}\n")
            f.write(f"f {b0} {t1} {t0}\n")
        tip_idx = 2 * n + 1
        for k in range(n):     # tip cap fan
            t0 = n + k + 1
            t1 = n + (k + 1) % n + 1
            f.write(f"f {t0} {t1} {tip_idx}\n")

    mat = ['        <make_named_material type="blinn" name="laser_beam" '
           f'diffuse="0 0 0" specular="0 0 0" emission="{BEAM_EMISSION}" />',
           '        <make_named_material type="glow" name="laser_glow" '
           f'emission="{GLOW_COLOR}" texture="{PREFIX}/textures/glow.png" />']
    # Glow billboard is listed AFTER the core so it composites over it.
    obj = ['        <object type="trianglemesh" material="laser_beam" '
           f'filename="{PREFIX}/obj/beam.obj" />',
           '        <object type="trianglemesh" material="laser_glow" '
           f'filename="{PREFIX}/obj/beam_glow.obj" />']
    light = ['        <light_source type="point" '
             f'i="{BEAM_LIGHT_I}" scale="1 1 1" '
             f'from="{nozzle[0]:.3f} {nozzle[1]:.3f} {nozzle[2]:.3f}" '
             'attenuation="1 0.15 0.25" />']
    return mat, obj, light


def load_parts():
    parts = []
    with open(os.path.join(HERE, "parts.tsv")) as f:
        for line in f:
            line = line.rstrip("\n")
            if not line:
                continue
            cols = line.split("\t")
            parts.append((cols[0], cols[1], cols[2] if len(cols) > 2 else ""))
    return parts


def make_stars():
    """Returns (material_lines, object_lines) for the starfield."""
    rng = random.Random(SEED)
    fwd = norm(sub(LOOK_AT, LOOK_FROM))
    right = norm(cross(fwd, UP))
    up2 = cross(right, fwd)

    half_v = math.tan(math.radians(FOVY / 2.0))
    aspect = W_RES / H_RES
    half_h = half_v * aspect
    margin = 1.08  # spill slightly past the frame edges

    names = [m[0] for m in STAR_MATERIALS]
    weights = [m[2] for m in STAR_MATERIALS]

    mat_lines = [
        f'        <make_named_material type="blinn" name="{n}" '
        f'diffuse="0 0 0" specular="0 0 0" emission="{e}" />'
        for (n, e, _w) in STAR_MATERIALS
    ]

    obj_lines = []
    for _ in range(N_STARS):
        sx = rng.uniform(-half_h, half_h) * margin
        sy = rng.uniform(-half_v, half_v) * margin
        d = norm((fwd[0] + sx * right[0] + sy * up2[0],
                  fwd[1] + sx * right[1] + sy * up2[1],
                  fwd[2] + sx * right[2] + sy * up2[2]))
        dist = rng.uniform(45.0, 95.0)
        px = LOOK_FROM[0] + d[0] * dist
        py = LOOK_FROM[1] + d[1] * dist
        pz = LOOK_FROM[2] + d[2] * dist
        radius = dist * rng.uniform(0.0007, 0.0021)  # ~1-3 px
        mat = rng.choices(names, weights=weights, k=1)[0]
        obj_lines.append(
            f'        <object type="sphere" material="{mat}" '
            f'radius="{radius:.4f}" center="{px:.3f} {py:.3f} {pz:.3f}" />')
    return mat_lines, obj_lines


def main():
    parts = load_parts()
    star_mats, star_objs = make_stars()
    beam_mats, beam_objs, beam_lights = build_beam()

    L = []
    L.append("<RT3>")
    L.append(f'    <lookat look_from="{LOOK_FROM[0]} {LOOK_FROM[1]} {LOOK_FROM[2]}" '
             f'look_at="{LOOK_AT[0]} {LOOK_AT[1]} {LOOK_AT[2]}" '
             f'up="{UP[0]} {UP[1]} {UP[2]}" />')
    L.append(f'    <camera type="perspective" fovy="{FOVY:g}" />')
    L.append(f'    <integrator type="blinn_phong" samples="{SAMPLES}" />')
    L.append('    <accelerator type="bvh" max_prims_per_node="4" />')
    L.append(f'    <film type="image" w_res="{W_RES}" h_res="{H_RES}" '
             'filename="renders/swordfish.png" img_type="png" '
             'caption="SEE YOU SPACE COWBOY..." '
             'caption_font="scenes/fonts/DejaVuSerif-Bold.ttf" '
             'caption_size="30" caption_color="0.95 0.95 0.92" />')
    L.append("")
    L.append("    <world_begin/>")
    # Subtle vertical gradient: near-black top, faint deep-blue bottom.
    L.append('        <background type="4_colors" '
             'tl="0.010 0.010 0.020" tr="0.010 0.010 0.020" '
             'bl="0.020 0.028 0.055" br="0.020 0.028 0.055" />')
    L.append("")
    L.append('        <light_source type="ambient" i="0.16 0.16 0.19" scale="1 1 1" />')
    L.append('        <light_source type="directional" i="0.9 0.88 0.82" '
             'scale="0.6 0.6 0.6" from="-3 5 -4" to="0 0 0" />')
    L.append('        <light_source type="directional" i="0.5 0.55 0.7" '
             'scale="0.4 0.4 0.4" from="4 2 5" to="0 0 0" />')
    L.append('        <light_source type="point" i="1 0.9 0.7" scale="1 1 1" '
             'from="2 3 -3" attenuation="1 0.02 0.008" />')
    if beam_lights:
        L.append("        <!-- red thrust light -->")
        L.extend(beam_lights)
    L.append("")
    L.append("        <!-- ===== starfield (emissive spheres) ===== -->")
    L.extend(star_mats)
    L.append("")
    L.extend(star_objs)
    L.append("")
    L.append("        <!-- ===== ship materials ===== -->")
    for matname, _obj, tex in parts:
        if tex:
            L.append(
                f'        <make_named_material type="blinn" name="{esc(matname)}" '
                f'diffuse="1 1 1" specular="0.15 0.15 0.15" glossiness="24" '
                f'texture="{esc(PREFIX + "/" + tex)}" />')
        else:
            L.append(
                f'        <make_named_material type="blinn" name="{esc(matname)}" '
                f'diffuse="0.6 0.6 0.62" specular="0.1 0.1 0.1" glossiness="16" />')
    L.append("")
    L.append("        <!-- ===== ship parts ===== -->")
    for matname, obj_rel, _tex in parts:
        L.append(
            f'        <object type="trianglemesh" material="{esc(matname)}" '
            f'filename="{esc(PREFIX + "/" + obj_rel)}" />')
    if beam_objs:
        L.append("")
        L.append("        <!-- ===== thrust laser beam ===== -->")
        L.extend(beam_mats)
        L.extend(beam_objs)
    L.append("    <world_end/>")
    L.append("</RT3>")

    with open(OUT, "w") as f:
        f.write("\n".join(L) + "\n")
    print(f"wrote {OUT}: {len(parts)} ship parts + {len(star_objs)} stars")


if __name__ == "__main__":
    main()
