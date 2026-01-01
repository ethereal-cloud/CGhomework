/**
 * code.cpp: 定义应用程序的入口点
 * 流体仿真系统的主程序
 */

#include "code.h"
#include "UI.h"

using namespace std;

/**
 * 主函数
 * 创建UI对象并启动仿真系统
 * @return 程序退出码
 */
int main()
{
	FluidSimulation::UI ui;
	ui.run();
	return 0;
}
