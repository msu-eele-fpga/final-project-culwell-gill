library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

entity pwm_controller is
	generic (
		CLK_PERIOD : time := 20 ns
	);
	port (
		clk : in std_logic;
		rst : in std_logic;
		period : in unsigned(16 - 1 downto 0);
		duty_cycle : in unsigned(12 - 1 downto 0);
		output : out std_logic
	);
end entity pwm_controller;

architecture pwm_controller_arch of pwm_controller is
	--Period: 16.10
	--Duty Cycle: 12.11
	signal count, one_period : unsigned(31 downto 0);
	signal counter_limit : unsigned(43 downto 0);
	signal done : boolean := false;
	
	begin
		one_period <= unsigned(((1 ms / CLK_PERIOD) / 1024) * period); --one period is 1 of period in seconds.
		counter_limit <= unsigned((duty_cycle * one_period) / 2048); --one duty cycle
		
		PERIOD_COUNTER : process(clk, rst)
		begin
			if (rst = '1') then
				count <= to_unsigned(1, 32);
				done <= false;
				output <= '1';
				
			elsif (rising_edge(clk) and count < one_period) then
				count <= count + 1;
				done <= false;
				
				if (count < counter_limit) then output <= '1';
				else output <= '0';
				end if;
				
			elsif (rising_edge(clk) and count >= one_period) then
				count <= to_unsigned(1, 32);
				done <= true;
				
			end if;
		end process;
end architecture;
