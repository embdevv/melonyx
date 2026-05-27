#!/usr/bin/env python3
"""
Particle Race Simulation - Programming Challenge 1
Simulates 4 particles racing from corners to the center (0, 0, 0) in a 700x700 screen
with constant acceleration.
"""

import math


class Particle:
    """Represents a particle with physics properties"""
    
    def __init__(self, name, start_pos, velocity, acceleration):
        self.name = name
        self.start_pos = start_pos
        self.velocity = velocity  # initial velocity in m/s
        self.acceleration = acceleration  # constant acceleration in m/s²
        self.distance = self._calculate_distance()
        
    def _calculate_distance(self):
        """Calculate 3D distance from starting position to center (0, 0, 0)"""
        x, y, z = self.start_pos
        return math.sqrt(x**2 + y**2 + z**2)
    
    def calculate_time_to_center(self):
        """
        Calculate time to reach center using kinematic equation:
        s = v₀*t + 0.5*a*t²
        Rearranged: 0.5*a*t² + v₀*t - s = 0
        Using quadratic formula to solve for t
        """
        a = 0.5 * self.acceleration
        b = self.velocity
        c = -self.distance
        
        discriminant = b**2 - 4*a*c
        
        if discriminant < 0:
            return None
        
        # Get the positive solution
        t1 = (-b + math.sqrt(discriminant)) / (2*a)
        t2 = (-b - math.sqrt(discriminant)) / (2*a)
        
        return t1 if t1 > 0 else (t2 if t2 > 0 else None)
    
    def calculate_final_velocity(self, time):
        """Calculate final velocity using v = v₀ + a*t"""
        return self.velocity + self.acceleration * time
    
    def calculate_average_velocity(self, time):
        """Calculate average velocity = (v₀ + v_final) / 2"""
        final_vel = self.calculate_final_velocity(time)
        return (self.velocity + final_vel) / 2
    
    def get_results(self):
        """Get all race results for this particle"""
        time = self.calculate_time_to_center()
        if time is None:
            return None
        
        final_velocity = self.calculate_final_velocity(time)
        average_velocity = self.calculate_average_velocity(time)
        
        return {
            'name': self.name,
            'time': time,
            'final_velocity': final_velocity,
            'average_velocity': average_velocity,
            'distance': self.distance
        }


def main():
    """Main simulation function"""
    
    # Define particles: (name, start_position, velocity, acceleration)
    particles = [
        Particle('Red', (-350, -350, 201), 80, 14.5),
        Particle('Green', (350, -350, 173), 90, 8),
        Particle('Blue', (350, 350, -300), 130, 1),
        Particle('Yellow', (-350, 350, -150), 110, 3)
    ]
    
    # Calculate results for each particle
    results = []
    for particle in particles:
        result = particle.get_results()
        if result:
            results.append(result)
    
    # Sort by time (winner is fastest)
    results.sort(key=lambda x: x['time'])
    
    # Display results
    print("=" * 80)
    print("PARTICLE RACE SIMULATION - PROGRAMMING CHALLENGE 1")
    print("=" * 80)
    print()
    
    for rank, result in enumerate(results, 1):
        print(f"🏁 RANK #{rank}: {result['name'].upper()}")
        print(f"   Time to finish:          {result['time']:.2f} seconds")
        print(f"   Velocity at finish line: {result['final_velocity']:.2f} m/s")
        print(f"   Average velocity:        {result['average_velocity']:.2f} m/s")
        print(f"   Distance covered:        {result['distance']:.2f} m")
        print()
    
    print("=" * 80)
    print("FINAL RANKING")
    print("=" * 80)
    for rank, result in enumerate(results, 1):
        print(f"{rank}. {result['name']:8} | Time: {result['time']:7.2f}s | "
              f"Finish Velocity: {result['final_velocity']:7.2f} m/s | "
              f"Avg Velocity: {result['average_velocity']:7.2f} m/s")
    
    print("=" * 80)


if __name__ == '__main__':
    main()
