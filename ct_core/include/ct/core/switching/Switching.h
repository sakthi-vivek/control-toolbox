/**********************************************************************************************************************
This file is part of the Control Toolbox (https://adrlab.bitbucket.io/ct), copyright by ETH Zurich, Google Inc.
Authors:  Michael Neunert, Markus Giftthaler, Markus Stäuble, Diego Pardo, Farbod Farshidian
Licensed under Apache2 license (see LICENSE file in main directory)
**********************************************************************************************************************/

#pragma once

namespace ct {
    namespace core {
        //! Declaring Switched type such that we can write Switched<SystemPtr>
        template <class T>
        using Switched = std::vector<T>;

        //! Describes a switch between phases
        template <class Phase, typename Time>
        struct SwitchEvent {
            std::shared_ptr<Phase> pre_phase;
            std::shared_ptr<Phase> post_phase;
            Time switch_time;
        };

        //! Describes a Phase sequence with timing
        /*!
         *  Each phase of the sequence has a start time, and end time
         *  Each event has a pre & post phase + switching time
         *
         *  Example:
         *  The following illustrates a sequence of 3 phases
         *  + ------- + ------- + ------- +
         *  t0   p0   t1   p1   t2   p2   t3
         *
         *  It contains 4 times to define the sequence.
         *  Two of those are switching times: t1 and t2
         *  There are two switching events: {p0, p1, t1} and {p1, p2, t2};
         *
         */
        template <class Phase, typename Time>
        class PhaseSequence {
        public:
            typedef std::shared_ptr<Phase> PhasePtr_t;
            typedef std::vector<PhasePtr> PhaseSchedule_t;
            typedef std::vector<Time> TimeSchedule_t;

            //! Construct empty sequence (with default start time)
            PhaseSequence(const Time& start_time = 0) {
              time_schedule_.push_back(start_time);
            }
            //! Destructor
            ~PhaseSequence() {}

            /// @brief add a phase with duration
            void addPhase(const PhasePtr_t& phase, const Time& duration) {
              phase_schedule_.push_back(phase);
              time_schedule_.emplace_back(time_schedule_.back() + duration);
            }
            /// @brief get number of phases
            std::size_t getNumPhases() { return phase_schedule_.size(); }
            /// @brief get number of switches
            std::size_t getNumSwitches() { return getNumPhases() - 1; }
            /// @brief get start time from sequence index
            Time getStartTimeFromIdx( std::size_t idx) { return time_schedule_[idx]; }
            /// @brief get end time from sequence index
            Time getEndTimeFromIdx( std::size_t idx) { return time_schedule_[idx+1]; }
            /// @brief get phase pointer from sequence index
            PhasePtr_t getPhasePtrFromIdx( std::size_t idx ) { return phase_schedule_[idx]; }
            /// @brief get phase pointer from time
            PhasePtr_t getPhasePtrFromTime( Time time ) { return getPhasePtrFromIdx(getIdxFromTime(time)); }
            /// @brief get next switch event from sequence index
            SwitchEvent<Phase, Time> getSwitchEventFromIdx( std::size_t idx) {
              return {getPhasePtr(idx), getPhasePtr(idx+1), getEndTimeFromIdx(idx)};
            }
            /// @brief get next switch event from time
            SwitchEvent<Phase, Time> getSwitchEventFromTime( Time time ) {
              return getSwitchEventFromIdx(getIdxFromTime(time));
            }
            /// @brief get sequence index from time
            std::size_t getIdxFromTime( Time time ) {
              // Finds pointer to first element less or equal to time
              // i.e. it returns the index for the phase with time in [t_start, t_end)
              auto low = std::lower_bound(time_schedule_.begin(), time_schedule_.end(), time);
              return low - time_schedule_.begin();
            }

        private:
            PhaseSchedule_t phase_schedule_;
            TimeSchedule_t time_schedule_;
        };

        using ContinuousModeSequence = PhaseSequence<std::size_t, double>;
        using DiscreteModeSequence = PhaseSequence<std::size_t, int>;

    }  // namespace core
}  // namespace ct