#pragma once

// Proveniência: copiado do Aquorbium (src/app/RealtimeAudioThread.h, commit
// 19140e2, mesmo autor, mesmo código escrito para o RASGO) em 2026-10-28 e
// adaptado ao namespace do Asterismus. Motivo: a thread de áudio do JUCE/ALSA
// roda sem prioridade (SCHED_OTHER) e acumulava ATRASOS mesmo com o app em
// STOP (513 em 9 h); ver docs/DIAGNOSTICO_2026-10-28.md.

#include <algorithm>

#if defined(__linux__)
#include <pthread.h>
#include <sched.h>
#include <sys/resource.h>
#endif

namespace rasgo::performance::app {

// Chamada uma vez, de dentro da própria thread de áudio. Pede SCHED_RR com a
// prioridade que o RLIMIT_RTPRIO do usuário permite, limitada a 70 para ficar
// abaixo do PipeWire. Devolve a prioridade obtida, ou 0 sem tempo real.
inline int promoteCurrentThreadToRealtime() noexcept {
#if defined(__linux__)
    rlimit limit {};
    if (getrlimit(RLIMIT_RTPRIO, &limit) != 0 || limit.rlim_cur == 0) return 0;
    const auto ceiling = static_cast<int>(std::min<rlim_t>(limit.rlim_cur, 70));
    const auto priority = std::clamp(ceiling, sched_get_priority_min(SCHED_RR),
                                     sched_get_priority_max(SCHED_RR));
    sched_param parameter {};
    parameter.sched_priority = priority;
    return pthread_setschedparam(pthread_self(), SCHED_RR, &parameter) == 0 ? priority : 0;
#else
    return 0;
#endif
}

} // namespace rasgo::performance::app
