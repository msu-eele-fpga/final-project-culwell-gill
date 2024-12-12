library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

entity rgb_led_controller is
	port(
		clk : in std_logic;
		rst : in std_logic;
		avs_read : in std_logic;
		avs_write : in std_logic;
		avs_address : in std_logic_vector(1 downto 0);
		avs_readdata : out std_logic_vector(31 downto 0);
		avs_writedata : in std_logic_vector(31 downto 0);
		rgb_output : out std_logic_vector(2 downto 0)
	);
end entity;

architecture rgb_led_controller_arch of rgb_led_controller is
	component pwm_controller is
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
	end component;
	
	signal period : unsigned(15 downto 0);
	signal red_duty_cycle : unsigned(11 downto 0);
	signal green_duty_cycle : unsigned(11 downto 0);
	signal blue_duty_cycle : unsigned(11 downto 0);
	
begin
	RED_PWM_CONTROLLER : pwm_controller
		generic map (CLK_PERIOD => 20 ns)
		port map (clk => clk, rst => rst, period => period, duty_cycle => red_duty_cycle, output => rgb_output(0));
	GREEN_PWM_CONTROLLER : pwm_controller
		generic map (CLK_PERIOD => 20 ns)
		port map (clk => clk, rst => rst, period => period, duty_cycle => green_duty_cycle, output => rgb_output(1));
	BLUE_PWM_CONTROLLER : pwm_controller
		generic map (CLK_PERIOD => 20 ns)
		port map (clk => clk, rst => rst, period => period, duty_cycle => blue_duty_cycle, output => rgb_output(2));
		
	avalon_register_read: process(clk)
	begin
		if rising_edge(clk) and avs_read = '1' then
			case avs_address is
				when "00" =>
					avs_readdata <= (others => '0');
					avs_readdata(15 downto 0) <= std_logic_vector(period);
				when "01" =>
					avs_readdata <= (others => '0');
					avs_readdata(11 downto 0) <= std_logic_vector(red_duty_cycle);
				when "10" =>
					avs_readdata <= (others => '0');
					avs_readdata(11 downto 0) <= std_logic_vector(green_duty_cycle);
				when "11" =>
					avs_readdata <= (others => '0');
					avs_readdata(11 downto 0) <= std_logic_vector(blue_duty_cycle);
				when others => avs_readdata <= (others => '0');
			end case;
		end if;
	end process;
	
	avalon_register_write: process(clk, rst)
	begin
		if rst = '1' then
			period <= to_unsigned(0, 16);
			red_duty_cycle <= to_unsigned(0, 12);
			green_duty_cycle <= to_unsigned(0, 12);
			blue_duty_cycle <= to_unsigned(0, 12);
			
		elsif rising_edge(clk) and avs_write = '1' then
			case avs_address is
				when "00" => period <= unsigned(avs_writedata(15 downto 0));
				when "01" => red_duty_cycle <= unsigned(avs_writedata(11 downto 0));
				when "10" => green_duty_cycle <= unsigned(avs_writedata(11 downto 0));
				when "11" => blue_duty_cycle <= unsigned(avs_writedata(11 downto 0));
				when others => null;
			end case;
		end if;
	end process;
end architecture;