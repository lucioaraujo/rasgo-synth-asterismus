#include <cmath>
#include <cstdlib>
#include <iostream>
#include <rasgo/performance/TubaBreathVoice.hpp>
int main(){ rasgo::performance::TubaBreathVoice v; v.prepare(48000,185); v.setFrequency(55); double e=0; for(int i=0;i<48000;i++){double s=v.process(.55,.45); if(!std::isfinite(s)){std::cerr<<"nonfinite\n";return 1;} e+=s*s;} if(e<=1e-6){std::cerr<<"silent\n";return 1;} std::cout<<"TubaBreathVoice tests passed\n"; }
