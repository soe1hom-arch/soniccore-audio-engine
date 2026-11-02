/*
 * Copyright 2026 SonicCore contributors
 * SPDX-License-Identifier: Apache-2.0
 */

package com.soniccore.android

/**
 * Conservative conflict detector for host UIs.
 *
 * It reports likely vendor DSP stacks so the host can ask the user to choose
 * one processing path. It does not disable, kill, or modify another package.
 */
object EffectConflictDetector {
    data class Conflict(val packageName: String, val label: String)

    private val known = mapOf(
        "com.dolby" to "Dolby audio stack",
        "com.dolby atmos" to "Dolby Atmos stack",
        "com.miui.misound" to "Xiaomi Mi Sound",
        "com.miui.audioeffect" to "Xiaomi audio effects",
        "com.huawei.audioeffect" to "Huawei audio effects",
        "com.dirac.acs" to "Dirac audio stack",
        "com.sonyericsson.soundenhancement" to "Sony sound enhancement"
    )

    fun inspect(packageNames: Iterable<String>): List<Conflict> = packageNames
        .mapNotNull { packageName ->
            val normalized = packageName.lowercase()
            known.entries.firstOrNull { normalized.contains(it.key) }
                ?.let { Conflict(packageName, it.value) }
        }
        .distinctBy { it.packageName }
}

