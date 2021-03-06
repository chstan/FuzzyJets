#!/usr/bin/python

import os, time, subprocess, pickle, threading, random

def safe_mkdir(p):
    if not os.path.isdir(p):
        os.makedirs(p)

def file_name(prefix, is_intermediate, size, lw, mu, rec, cut, ejw, off, PP, TBS, noise):
    lw_flag = "1" if lw else "0"
    rec_flag = "1" if rec else "0"
    ejw_str = "0" if (ejw < 0) else str(int(ejw * 1000))
    div = "/temp_" if is_intermediate else "/"
    return prefix + div + str(size) + "s_" + str(mu) + "mu_" + lw_flag + "lw_" + rec_flag + "rec_" + str(cut) + "cut_" + ejw_str + "ejw_" + str(off) + "off_" + str(PP) + "PP_" + str(TBS) + "TBS_" + str(int(noise * 100)) + "SN"

#def subprocess_call_bg(args):
#    subprocess.call(args)

def submit_fuzzy(mu, size, lw, rec, cut, ejw, off, PP, TBS, noise, n_events, i, unique_id):
    logdir = cwd + 'logs/' + name + "/" + time_postfix + "_bsub_" + str(mu) + "_" + str(unique_id)
    safe_mkdir(logdir)

    scratchdir = "/scratch/chstan/" + time_postfix + "_" + str(unique_id)
    safe_mkdir(scratchdir)
    lw_flag = "1" if lw else "0"
    rec_flag = "1" if rec else "0"
    outfile_name = file_name(scratchdir, True, size, lw, mu, rec, cut, ejw, off, PP, TBS, noise) + "_" + str(i) + ".root"
    size_s = str(1.0*size / 10)
    submit = ['bsub', '-q', queue,
              '-R', 'select[(!preempt&&rhel60&&cvmfs&&inet)]',
              '-o', logdir + "/" + str(i) + ".log",
              subfile, workdir, scratchdir, outdir,
              "./Fuzzy", "--NEvents", str(n_events), "--OutFile", outfile_name,
              "--NPV", str(mu),  "--Size", str(size_s),
              "--PythiaConfig", pythia_conf,
              "--LearnWeights", lw_flag, "--Recombination", rec_flag,
              "--pTMin", str(cut), "--Batch", "1", "--EventJetStrength", str(ejw),
              "--RhoOffset", str(off), "--FinalRecombination", str(PP),
              "--TowerSubtraction", str(TBS),
              "--SeedNoise", str(noise)]
    #print " ".join(submit)
    time.sleep(random.randint(delay_secs_low, delay_secs_high))
    subprocess.call(submit)

workdir = "/u/at/chstan/nfs/summer_2014/ForConrad/"

pythia_conf = workdir + "configs/default_batch.pythia"

cwd = os.getcwd() + "/"
subfile = cwd + "_batchSingleSub.sh"

time_postfix = time.strftime('%Y_%m_%d_%Hh%Mm%Ss')

events_per_job = 10000
n_jobs = 10
n_jobs_patch = 2
delay_secs_low = 0
delay_secs_high = 0
queue = 'xlong'

name = '100k_zprime_noise'
outdir = cwd + 'files/' + name + '/' + time_postfix
safe_mkdir(outdir)

PPs = [0] #final distance merger?
TBSs = [0] #tower based subtraction?
NPVs = [0] #[0, 20, 40]
sizes = [10]
learnWeights = [False]
recombinations = [False]
pT_cuts = [5] #[5, 10, 15, 20, 30, 50]
event_jet_weights = [-1] #[-1, 0.001, 0.005, 0.01, 0.02, 0.04] # should be decimal
event_jet_offsets = [0]   #[-5, 0, 5] # should be large decimal (integralish)
seed_noises = [0, 0.2, 1.0] # should be decimal

j = 0
cleanup_commands = []

for current_noise in seed_noises:
    for current_TBS in TBSs:
        for current_PP in PPs:
            for current_mu in NPVs:
                for current_size in sizes:
                    for current_lw in learnWeights:
                        for current_rec in recombinations:
                            for current_pT_cut in pT_cuts:
                                for current_ejw in event_jet_weights:
                                    for current_offset in event_jet_offsets:
                                        out_base = file_name(outdir, False, current_size, current_lw, current_mu, current_rec, current_pT_cut, current_ejw, current_offset, current_PP, current_TBS, current_noise)
                                        cleanup_base = file_name(outdir, True, current_size, current_lw, current_mu, current_rec, current_pT_cut, current_ejw, current_offset, current_PP, current_TBS, current_noise)
                                        cleanup_command = "hadd " + out_base + ".root" + " " \
                                            + cleanup_base + "_{0.." + str(n_jobs-1) + "}.root"
                                        cleanup_commands.append(cleanup_command)
                                        for current_job in range(n_jobs):
                                            submit_fuzzy(current_mu, current_size, current_lw, current_rec, current_pT_cut, current_ejw, current_offset, current_PP, current_TBS, current_noise, events_per_job, current_job, j)
                                            j += 1

outdir = cwd + 'files/' + name + '_patch/' + time_postfix
safe_mkdir(outdir)

for current_noise in seed_noises:
    for current_TBS in TBSs:
        for current_PP in PPs:
            for current_mu in NPVs:
                for current_size in sizes:
                    for current_lw in learnWeights:
                        for current_rec in recombinations:
                            for current_pT_cut in pT_cuts:
                                for current_ejw in event_jet_weights:
                                    for current_offset in event_jet_offsets:
                                        for current_job in range(n_jobs_patch):
                                            submit_fuzzy(current_mu, current_size, current_lw, current_rec, current_pT_cut, current_ejw, current_offset, current_PP, current_TBS, current_noise, events_per_job, current_job, j)
                                            j += 1

with open('clean_scripts/' + time_postfix + '.clscr', 'wb') as outf:
    pickle.dump(cleanup_commands, outf)

print "SUBMITTED " + str(len(seed_noises) * len(TBSs) * len(PPs) * len(NPVs) * len(learnWeights) * len(sizes) * (n_jobs + n_jobs_patch) * len(recombinations) * len(pT_cuts) * len(event_jet_weights) * len(event_jet_offsets)) + " JOBS TO THE QUEUE " + queue
